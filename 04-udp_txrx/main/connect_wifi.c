/**
 * This file is part of espidf-udp.
 *
 * espidf-udp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * espidf-udp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with espidf-udp.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright 2020-2021 Pascal Bodin
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "lwip/errno.h"
#include "lwip/sockets.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "messages.h"
#include "send_datagram.h"
#include "supervisor.h"
#include "utilities.h"

#define INPUT_QUEUE_LENGTH 3

#define CONNECT_RETRY_PERIOD_MS CONFIG_UDP_RETRY_PERIOD_MS

#define DEST_IPV4_ADDR CONFIG_UDP_IPV4_ADDR
#define DEST_PORT CONFIG_UDPDESTINATION_PORT

static const char *TAG = "CW";

// Input queue.
QueueHandle_t cw_input_queue;

typedef enum {
	CW_WAIT_CONNECT_MSG_ST,
	CW_WAIT_STA_ST,
	CW_WAIT_IP_ST,
	CW_WAIT_AND_CONNECT_ST,
	CW_WAIT_DISCONNECT_MSG_ST,
	CW_ERROR_ST
} state_t;

static state_t current_state;

/**
 * Event handler for events generated by the Wi-Fi task and the LwIP task.
 */
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {

	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
	    message_t message_to_send;
	    message_to_send.message = CW_STA_OK;
	    message_to_send.no_payload.nothing = 0;
	    BaseType_t rs = send_to_queue(cw_input_queue, &message_to_send, TAG);
	    if (rs != pdTRUE) {
	    	ESP_LOGE(TAG, "Error on sending message to myself - %d", rs);
	    }
	    return;
	}
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		// We were not able to connect, or we were connected and got disconnected.
	    message_t message_to_send;
	    message_to_send.message = CW_AP_NOK;
	    message_to_send.no_payload.nothing = 0;
	    BaseType_t rs = send_to_queue(cw_input_queue, &message_to_send, TAG);
	    if (rs != pdTRUE) {
	    	ESP_LOGE(TAG, "Error on sending message to myself - %d", rs);
	    }
	    return;
	}
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		// We are connected, and got an IP address.
	    message_t message_to_send;
	    message_to_send.message = CW_IP_OK;
	    message_to_send.no_payload.nothing = 0;
	    BaseType_t rs = send_to_queue(cw_input_queue, &message_to_send, TAG);
	    if (rs != pdTRUE) {
	    	ESP_LOGE(TAG, "event_handler - error on sending message to myself - %d", rs);
	    }
	    return;
	}
}

/**
 * Returns true if Wi-Fi initialization is OK, false otherwise.
 */
static bool init_wifi(void) {

	esp_err_t esp_rs;

	esp_netif_create_default_wifi_sta();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_rs = esp_wifi_init(&cfg);
	if (esp_rs != ESP_OK) {
		ESP_LOGE(TAG, "Error from esp_wifi_init: %d", esp_rs);
		return false;
	}
	// Register our event_handle that will receive Wi-Fi task events.
	esp_rs = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
	if (esp_rs != ESP_OK) {
		ESP_LOGE(TAG, "Error from esp_event_handler_register: %d", esp_rs);
		return false;
	}
	// And register the same event handler to receive the event telling that an
	// IP address has been assigned. esp_netif will automatically start the DHCP client
	// once the Wi-Fi task is connected to the AP.
	esp_rs = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);
	if (esp_rs != ESP_OK) {
		ESP_LOGE(TAG, "Error from esp_event_handler_register: %d", esp_rs);
		return false;
	}
	esp_rs = esp_wifi_set_mode(WIFI_MODE_STA);
	if (esp_rs != ESP_OK) {
		ESP_LOGE(TAG, "Error from esp_wifi_set_mode: %d", esp_rs);
		return false;
	}
	// At this stage, Wi-Fi initialization is OK.
	return true;

}

/**
 * Event handler for the timer.
 */
static void timer_handler(TimerHandle_t timer) {

	message_t message_to_send;
	message_to_send.message = CW_TIMEOUT;
	message_to_send.no_payload.nothing = 0;
    BaseType_t rs = send_to_queue(cw_input_queue, &message_to_send, TAG);
    if (rs != pdTRUE) {
    	ESP_LOGE(TAG, "timer_handler - error on sending message to myself - %d", rs);
    }

}

void connect_wifi_task(void *pvParameters) {

	// Delay used for xTicksToWait when calling xQueueReceive().
	const TickType_t delay_60s = pdMS_TO_TICKS(60000);

	// Delay used for xTicksToWait when calling xTimerStart().
	const TickType_t delay_500ms = pdMS_TO_TICKS(500);

	TimerHandle_t timer = NULL;

	struct sockaddr_in dest_addr;
	int sock = 0;

	BaseType_t fr_rs;  // Return status for FreeRTOS calls.
	esp_err_t esp_rs;  // Return status for ESP-IDF calls.

	message_t received_message;
	message_t message_to_send;

	current_state = CW_WAIT_CONNECT_MSG_ST;

	// Prepare UDP context.
	ESP_LOGI(TAG, "Preparing for sending datagrams to %s - %d", DEST_IPV4_ADDR, DEST_PORT);
	int addr_family = AF_INET;
	int ip_protocol = IPPROTO_IP;
	int rs = inet_aton(DEST_IPV4_ADDR, &dest_addr.sin_addr.s_addr);
	if (rs == 0) {
		ESP_LOGE(TAG, "Incorrect IPv4 address: %s", DEST_IPV4_ADDR);
		send_error(CW_INIT_ERR, TAG);
		current_state = CW_ERROR_ST;
	}
	if (current_state != CW_ERROR_ST) {
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(DEST_PORT);
		sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (sock < 0) {
			ESP_LOGE(TAG, "Error from socket: %d", sock);
			send_error(CW_INIT_ERR, TAG);
			current_state = CW_ERROR_ST;
		}
	}

	// Create our input queue.
	cw_input_queue = xQueueCreate(INPUT_QUEUE_LENGTH, sizeof(message_t));
	if (cw_input_queue == 0) {
		ESP_LOGE(TAG, "Error from xQueueCreate");
		send_error(CW_INIT_ERR, TAG);
		current_state = CW_ERROR_ST;
	}

	// Create the timer we'll use to reconnect to the access point.
	if (current_state != CW_ERROR_ST) {
		uint8_t timerID = 0;
		timer = xTimerCreate("CW_TIMER",
				pdMS_TO_TICKS(CONNECT_RETRY_PERIOD_MS),
				pdFALSE,  // uxAutoReload.
				&timerID,
				timer_handler);
		if (timer == NULL) {
			ESP_LOGE(TAG, "Error from xTimerCreate");
			send_error(CW_INIT_ERR, TAG);
			current_state = CW_ERROR_ST;
		}
	}

	// Initialize Wi-Fi.
	if (current_state != CW_ERROR_ST) {
		bool bool_rs = init_wifi();
		if (!bool_rs) {
			// Error in initialization. Inform the supervisor.
			send_error(CW_INIT_ERR, TAG);
			current_state = CW_ERROR_ST;
		}
	}

	while (true) {

		// Wait for an incoming message.
		fr_rs = xQueueReceive(cw_input_queue, &received_message, delay_60s);
		if (fr_rs != pdTRUE) {
			// Timeout. Go back to receive.
			ESP_LOGI(TAG, "Queue receive timeout");
			continue;
		}

		switch (current_state) {

		case CW_WAIT_CONNECT_MSG_ST:
			if (received_message.message != CW_CONNECT) {
				// Unexpected message, ignore it, stay in this state.
				ESP_LOGE(TAG, "CW_CONNECT_ST - unexpected message received: %d", received_message.message);
				break;
			}
			ESP_LOGI(TAG, "CW_WAIT_CONNECT_MSG_T - connect message received");
			// At this stage, connect message was received.
			ESP_LOGI(TAG, "AP SSID: %s", received_message.cw_connect.ssid);
			// Try to connect to related access point.
			wifi_config_t wifi_config;
			strncpy((char *)wifi_config.sta.ssid, received_message.cw_connect.ssid, 32);
			strncpy((char *)wifi_config.sta.password, received_message.cw_connect.password, 64);
			wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK,
			wifi_config.sta.pmf_cfg.capable = true;
			wifi_config.sta.pmf_cfg.required = false;
			esp_rs = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
			if (esp_rs != ESP_OK) {
				ESP_LOGE(TAG, "Error from esp_wifi_set_config: %d", esp_rs);
				send_error(CW_START_ERR, TAG);
				current_state = CW_ERROR_ST;
				break;
			}
			esp_rs = esp_wifi_start();
			if (esp_rs != ESP_OK) {
				ESP_LOGE(TAG, "Error from esp_wifi_start: %d", esp_rs);
				send_error(CW_START_ERR, TAG);
				current_state = CW_ERROR_ST;
				break;
			}
			// Now, we wait for the WIFI_EVENT_STA_START event (see the event handler).
			current_state = CW_WAIT_STA_ST;
			break;

		case CW_WAIT_STA_ST:
			if (received_message.message != CW_STA_OK) {
				// Unexpected message, ignore it, stay in this state.
				ESP_LOGE(TAG, "CW_WAIT_STA_ST - unexpected message received: %d", received_message.message);
				break;
			}
			ESP_LOGI(TAG, "CW_WAIT_STA_ST - STA started");
			esp_rs = esp_wifi_connect();
			if (esp_rs != ESP_OK) {
				ESP_LOGE(TAG, "Error from esp_wifi_connect: %d", esp_rs);
				send_error(CW_CONNECT_ERR, TAG);
				current_state = CW_ERROR_ST;
				break;
			}
			// Now, we wait either for IP_EVENT_STA_GOT_IP or for WIFI_EVENT_STA_DISCONNECTED.
			current_state = CW_WAIT_IP_ST;
			break;

		case CW_WAIT_IP_ST:
			if (received_message.message == CW_AP_NOK) {
				// Connection to the AP failed.
				// Can we still retry?
				ESP_LOGI(TAG, "CW_WAIT_IP_ST - connection failed");
				// Wait for some time before retrying. The event handler
				// called at timer timeout will send the CW_TIMEOUT message.
				fr_rs = xTimerStart(timer, delay_500ms);
				if (fr_rs != pdPASS) {
					ESP_LOGE(TAG, "Error from xTimerStart: %d", fr_rs);
					send_error(CW_TIMER_ERR, TAG);
					current_state = CW_ERROR_ST;
					break;
				}
				current_state = CW_WAIT_AND_CONNECT_ST;
				break;
			}
			if (received_message.message == CW_IP_OK) {
				// Connection to the AP succeeded and we got an IP address.
				ESP_LOGI(TAG, "CW_WAIT_IP_ST - got an IP address");
				message_to_send.message = SD_CONNECTION_STATUS;
				message_to_send.sd_connection_status.connected = true;
				fr_rs = send_to_queue(sd_input_queue, &message_to_send, TAG);
				if (fr_rs != pdTRUE) {
					ESP_LOGE(TAG, "Error on sending message to send_datagram - %d", fr_rs);
					send_error(CW_QUEUE_ERR, TAG);
					current_state = CW_ERROR_ST;
					break;
				}
				current_state = CW_WAIT_DISCONNECT_MSG_ST;
				break;
			}
			// At this stage, we got another type of message.
			ESP_LOGE(TAG, "CW_WAIT_STA_ST - unexpected message received: %d", received_message.message);
			break;

		case CW_WAIT_AND_CONNECT_ST:
			if (received_message.message != CW_TIMEOUT) {
				// Unexpected message, ignore it, stay in this state.
				ESP_LOGE(TAG, "CW_WAIT_AND_CONNECT_ST - unexpected message received: %d", received_message.message);
				break;
			}
			// At this stage, we can try to reconnect.
			ESP_LOGI(TAG, "CW_WAIT_AND_CONNECT_ST - trying to reconnect");
			esp_rs = esp_wifi_connect();
			if (esp_rs != ESP_OK) {
				ESP_LOGE(TAG, "Error from esp_wifi_connect: %d", esp_rs);
				send_error(CW_CONNECT_ERR, TAG);
				current_state = CW_ERROR_ST;
				break;
			}
			// Now, we wait either for IP_EVENT_STA_GOT_IP or for WIFI_EVENT_STA_DISCONNECTED.
			current_state = CW_WAIT_IP_ST;
			break;

		case CW_WAIT_DISCONNECT_MSG_ST:
			if (received_message.message == CW_DISCONNECT) {
				esp_rs = esp_wifi_disconnect();
				if (esp_rs != ESP_OK) {
					ESP_LOGE(TAG, "Error from esp_wifi_disconnect: %d", esp_rs);
					send_error(CW_DISCONNECT_ERR, TAG);
					current_state = CW_ERROR_ST;
					break;
				}
				current_state = CW_WAIT_CONNECT_MSG_ST;
			}
			if (received_message.message == CW_AP_NOK) {
				// We got disconnected. Inform send_datagram task.
				ESP_LOGI(TAG, "CW_WAIT_DISCONNECT_ST - disconnected");
				message_to_send.message = SD_CONNECTION_STATUS;
				message_to_send.sd_connection_status.connected = false;
				fr_rs = send_to_queue(sd_input_queue, &message_to_send, TAG);
				if (fr_rs != pdTRUE) {
					ESP_LOGE(TAG, "Error on sending message to send_datagram - %d", fr_rs);
					send_error(CW_QUEUE_ERR, TAG);
					current_state = CW_ERROR_ST;
					break;
				}
				// Wait a bit and try to reconnect.
				fr_rs = xTimerStart(timer, delay_500ms);
				if (fr_rs != pdPASS) {
					ESP_LOGE(TAG, "Error from xTimerStart: %d", fr_rs);
					send_error(CW_TIMER_ERR, TAG);
					current_state = CW_ERROR_ST;
					break;
				}
				current_state = CW_WAIT_AND_CONNECT_ST;
				break;
			}
			if (received_message.message == CW_SEND_DATAGRAM) {
				// Send datagram.
				uint16_t payload_length = received_message.cw_send_datagram.payload_length;
				ESP_LOGI(TAG, "Sending a datagram - %d", payload_length);
				int err = sendto(sock, received_message.cw_send_datagram.payload,
						         payload_length, 0, (struct sockaddr *)&dest_addr,
								 sizeof(dest_addr));
				if (err < 0) {
					// Error on send. Inform send_datagram task.
					ESP_LOGE(TAG, "Error from sendto: %d", errno);
					message_to_send.message = SD_SEND_ERROR;
					message_to_send.sd_send_error.error = CW_SEND_ERR;
					fr_rs = send_to_queue(sd_input_queue, &message_to_send, TAG);
					if (fr_rs != pdTRUE) {
						ESP_LOGE(TAG, "Error on sending message to send_datagram - %d", fr_rs);
						send_error(CW_QUEUE_ERR, TAG);
						current_state = CW_ERROR_ST;
						break;
					}
					break;
				}
				// Stay in same state.
				break;
			}
			// At this stage, unexpected message.
			ESP_LOGE(TAG, "CW_WAIT_DISCONNECT_ST - unexpected message received: %d", received_message.message);
			break;

		case CW_ERROR_ST:
			// Once we enter this state, we stay in it.
			ESP_LOGI(TAG, "CW_ERROR_ST");
			break;

		default:
			ESP_LOGE(TAG, "Unknown state: %d", current_state);
			send_error(CW_UKNOWN_STATE_ERR, TAG);
			current_state = CW_ERROR_ST;
		}

	}
}

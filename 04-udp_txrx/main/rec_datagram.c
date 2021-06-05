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

#include <stdbool.h>

#include "lwip/errno.h"
#include "lwip/sockets.h"

#include "esp_log.h"

#include "messages.h"
#include "utilities.h"

#define REC_PORT CONFIG_UDPRECEPTION_PORT
#define LOCAL_IPV4_ADDR "0.0.0.0"

#define REC_DATA_MAX_LENGTH 100
#define IP_ADDR_MAX_LENGTH (15 + 1)

static const char *TAG = "RD";

static char addr_str[IP_ADDR_MAX_LENGTH];

typedef enum {
	RD_WAIT_DATA_ST,
	RD_ERROR_ST,
} state_t;

static state_t current_state;

// Reception buffer. In this sample applicaiton, we handle ASCII characters
// only. But we could exchange binary data as well. In this case, buffer
// type would be uint8_t[].
static char rec_buffer[REC_DATA_MAX_LENGTH];

void rec_datagram_task(void *pvParameters) {

	struct sockaddr_in rec_addr;
	int sock = 0;

    struct sockaddr_storage source_addr;
    socklen_t socklen;

	current_state = RD_WAIT_DATA_ST;

	// Prepare UDP context.
	ESP_LOGI(TAG, "Preparing for receiving datagrams on port %d", REC_PORT);
	int addr_family = AF_INET;
	int ip_protocol = IPPROTO_IP;
	int rs = inet_aton(LOCAL_IPV4_ADDR, &rec_addr.sin_addr.s_addr);
	if (rs == 0) {
		ESP_LOGE(TAG, "Incorrect IPv4 address: %s", LOCAL_IPV4_ADDR);
		send_error(RD_INIT_ERR, TAG);
		current_state = RD_ERROR_ST;
	}
	if (current_state != RD_ERROR_ST) {
		rec_addr.sin_family = AF_INET;
		rec_addr.sin_port = htons(REC_PORT);
		sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (sock < 0) {
			ESP_LOGE(TAG, "Error from socket: %d", sock);
			send_error(RD_INIT_ERR, TAG);
			current_state = RD_ERROR_ST;
		}
	}
	if (current_state != RD_ERROR_ST) {
		rs = bind(sock, (struct sockaddr *)&rec_addr, sizeof(rec_addr));
		if (rs < 0) {
			ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
			send_error(RD_INIT_ERR, TAG);
			current_state = RD_ERROR_ST;
		}
	}

	// Beware: in this sample application, this task is very simple, and does not
	// need to receive any incoming message. Consequently, it does not wait on the
	// incoming message queue but on the reception of datagram.
	// If there is an error while in RD_WAIT_DATA_ST state, the task will transition
	// to the error state, and consequently will enter an infinite loop. We let
	// FreeRTOS handle this :-) In a real application, we should do better!
	while (true) {

		switch (current_state) {

		case RD_WAIT_DATA_ST:
            socklen = sizeof(source_addr);
            // Blocking call.
            int len = recvfrom(sock, rec_buffer, sizeof(rec_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d - datagram ignored", errno);
                break;
            }
            // At this stage, we received a datagram. Check sender.
            if (source_addr.ss_family == PF_INET) {
                 inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
            }
            rec_buffer[len] = 0;
            ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
            ESP_LOGI(TAG, "%s", rec_buffer);
            break;

		case RD_ERROR_ST:
			// Once we enter this state, we stay in it.
			ESP_LOGI(TAG, "RD_ERROR_ST");
			break;

		default:
			ESP_LOGE(TAG, "Unknown state: %d", current_state);
			send_error(RD_UKNOWN_STATE_ERR, TAG);
			current_state = RD_ERROR_ST;

		}

	}

}

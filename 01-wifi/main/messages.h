/**
 * Structures of messages that can be sent to/by tasks.
 *
 * Copyright 2021 Pascal Bodin
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include <stdint.h>

#define DATA_MAX_LENGTH 100

// Types of messages.
typedef enum {
	MSG_WIFI_START,			// Start Wi-Fi connection to configured AP.
	MSG_WIFI_STOP,			// Stop Wi-Fi.
	MSG_WIFI_SEND_DATA,		// Send location and optional data.
	MSG_WIFI_AP_TO,			// AP scan timeout.
	MSG_WIFI_ACK_TO,		// ACK timeout.
	MSG_WIFI_STA_OK,		// STA state OK.
	MSG_WIFI_SCAN_DONE,		// Scan done.
	MSG_WIFI_STA_STOP,		// STA stopped.
	MSG_WIFI_STA_KO,		// Disconnected from AP.
	MSG_WIFI_IP_OK,			// Got an IP address.
	MSG_WIFI_UNSUPP_EVENT,	// Unsupported event.
} msg_type_t;

// MSG_WIFI_SEND_DATA message payload.
typedef struct {
	uint8_t length;
	uint8_t data[DATA_MAX_LENGTH];
} msg_send_data_payload_t;

// Message structure.
typedef struct {
	msg_type_t msg_type;
	union {
		msg_send_data_payload_t send_loc_payload;
	} payload;
} msg_t;

#endif /* MESSAGES_H_ */

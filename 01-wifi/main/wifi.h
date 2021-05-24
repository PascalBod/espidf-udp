/**
 * Interface to the wifi task.
 *
 * Copyright 2021 Pascal Bodin
 */

#ifndef WIFI_H_
#define WIFI_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum {
	WIFI_OK,
} wifi_status_t;

/**
 * Returns true if Wi-Fi initialization is OK, false otherwise.
 *
 * This is a blocking function.
 *
 * Wi-Fi must not be initialized when this function is called.
 *
 * Input parameters:
 * - input_queue: message queue where to send events
 *
 * Returned value:
 * - WIFI_OK: Wi-Fi initialization performed
 */
wifi_status_t init_wifi(QueueHandle_t input_queue);

/**
 * Must be called before calling init_wifi() another time.
 */
wifi_status_t stop_wifi(void);

#endif /* WIFI_H_ */

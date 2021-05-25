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
	WIFI_INIT_ERR,
	WIFI_HND_REG_ERR,
	WIFI_STA_ERR,
	WIFI_GET_CNTR_ERR,
	WIFI_SET_CNTR_ERR,
	WIFI_START_ERR,
	WIFI_SET_PWR_ERR,
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
 * - WIFI_INIT_ERR: error while initializing Wi-Fi
 * - WIFI_HND_REG_ERR: error on handler registration
 * - WIFI_STA_ERR: error when entering station mode
 * - WIFI_GET_CNTR_ERR: error when reading country configuration
 * - WIFI_SET_CNTR_ERR: error on setting country configuration
 * - WIFI_START_ERR: error on starting Wi-Fi
 * - WIFI_SET_PWR_ERR: error on setting max power
 */
wifi_status_t init_wifi(QueueHandle_t input_queue);

/**
 * Must be called before calling init_wifi() another time.
 *
 * Returned value:
 * - WIFI_OK: Wi-Fi stopped
 */
wifi_status_t stop_wifi(void);

#endif /* WIFI_H_ */

/**
 * Communication task interface.
 *
 * Copyright 2021 Pascal Bodin
 */
#ifndef COMM_TASK_H_
#define COMM_TASK_H_

#include <stdint.h>

/**
 * FreeRTOS task that handles Wi-Fi.
 *
 * Accepted messages:
 * - MSG_WIFI_START
 * - MSG_WIFI_SEND_DATA
 * - MSG_WIFI_STOP
 */
void comm_task(void *pvParameters);

#endif  /* COMM_TASK_H_ */

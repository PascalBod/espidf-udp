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
 * Copyright 2020 Pascal Bodin
 */

#ifndef MAIN_CONNECT_WIFI_H_
#define MAIN_CONNECT_WIFI_H_

#include "freertos/queue.h"

extern QueueHandle_t cw_input_queue;

void connect_wifi_task(void *pvParameters);

#endif /* MAIN_CONNECT_WIFI_H_ */

/**
 * Copyright 2021 Pascal Bodin
 */

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi.h"

#define TEST_PERIOD_MS 20000

static const char *VERSION = "0.1.0";

static const char *TAG = "APP";

void app_main(void)
{

	ESP_LOGI(TAG, "Version: %s", VERSION);

    //Initialize NVS. Required for esp_wifi_init().
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize TCP/IP stack. Required for Wi-Fi.
    ESP_ERROR_CHECK(esp_netif_init());

    // Do not exit from app_main().
    while (true) {
    	vTaskDelay(pdMS_TO_TICKS(TEST_PERIOD_MS));
    	ESP_LOGI(TAG, "End of message wait period");
    }

}

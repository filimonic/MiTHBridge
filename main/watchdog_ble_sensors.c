#include "watchdog_ble_sensors.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_system.h"

#ifdef CONFIG_MITHBRIDGE_BLE_WATCHDOG_TIMEOUT_MIN
    #define MITHBRIDGE_BLE_WATCHDOG_TIMEOUT_US (CONFIG_MITHBRIDGE_BLE_WATCHDOG_TIMEOUT_MIN * 60 * 1000000)
#else
    #define MITHBRIDGE_BLE_WATCHDOG_TIMEOUT_US (30 * 60 * 1000000)
#endif

static volatile int64_t watchdog_feed_time;

void watchdog_ble_sensors_task(void *pvParameters)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(30000));
        int64_t now = esp_timer_get_time();
        if ((now - watchdog_feed_time) > (int64_t)MITHBRIDGE_BLE_WATCHDOG_TIMEOUT_US) 
        {
            esp_restart();
        }
    }
}

void watchdog_ble_sensors_feed()
{
    watchdog_feed_time = esp_timer_get_time();
}


esp_err_t watchdog_ble_sensors_start()
{
#if CONFIG_MITHBRIDGE_BLE_WATCHDOG_ENABLED
    watchdog_ble_sensors_feed();
    return xTaskCreate(watchdog_ble_sensors_task, "wd_ble_sens", 3072, NULL, 1, NULL) == pdPASS ? ESP_OK : ESP_FAIL;
#endif
    return ESP_OK;
}
#include "build_checks.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_private/usb_console.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "led_flasher.h"
#include "measurement.h"
#include "sensors_ble.h"
#include "sensors_self.h"
#include "output.h"
#include "setup_gpio.h"
#include "app_desc.h"
#include "watchdog_ble_sensors.h"

// static const char * TAG = "BLE_SCAN";

void init_nvs()
{
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void init_usb_console()
{
    
}

int println_fn(void *line)
{
    return printf("%s\n", (const char *)line);
}

void app_main(void)
{
    init_nvs();
    setup_gpio();
    ESP_ERROR_CHECK(led_flasher_start());
    ESP_ERROR_CHECK(output_start(println_fn));
    ESP_ERROR_CHECK(sensors_self_start(output_enqueue_measurement));
    ESP_ERROR_CHECK(sensor_ble_start(output_enqueue_measurement, watchdog_ble_sensors_feed));
    ESP_ERROR_CHECK(watchdog_ble_sensors_start());
}

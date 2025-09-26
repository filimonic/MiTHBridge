#include "sensors_self.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#if CONFIG_SENSORS_SELF_ENABLED
#include "driver/temperature_sensor.h"
#include "measurement.h"
#include "string.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "device_mac.h"

temperature_sensor_handle_t temperature_sensor;


void sensors_self_task(void *pvParameters)
{
    output_enqueue_measurement_fn_t measurement_put_fn = (output_enqueue_measurement_fn_t)pvParameters;
    measurement_t measurement;
    measurement.kind = MEASUREMENT_KIND_ESP;
    memcpy(measurement.data_esp.mac, device_mac_get_8(), sizeof(measurement.data_esp.mac));
    measurement.data_esp.counter = 0;

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(SENSORS_SELF_INTERVAL_MS)); 

        measurement.data_esp.uptime_seconds = esp_timer_get_time() / (int64_t)1E6;

        // Read temperature
        float temperature_c;

        if (temperature_sensor_get_celsius(temperature_sensor, &temperature_c) == ESP_OK)
        {
            measurement.data_esp.temperature_c = temperature_c;
        }
        measurement_put_fn(&measurement, false);
        measurement.data_esp.counter++;
    }
}
#endif // #if CONFIG_SENSORS_SELF_ENABLED

esp_err_t sensors_self_start(output_enqueue_measurement_fn_t measurement_put_fn)
{
#if CONFIG_SENSORS_SELF_ENABLED
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temperature_sensor));
    ESP_ERROR_CHECK(temperature_sensor_enable(temperature_sensor));
    return xTaskCreate(sensors_self_task, "self_sensors", 3072, (void *)measurement_put_fn, 5, NULL) == pdPASS ? ESP_OK : ESP_FAIL;
#endif
    return ESP_OK;
}
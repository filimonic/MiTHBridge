#include "sensors_self.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "measurement.h"
#include "string.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "device_mac.h"



void keepalive_task(void *pvParameters)
{

    output_enqueue_measurement_fn_t measurement_put_fn = (output_enqueue_measurement_fn_t)pvParameters;
    measurement_t measurement;
    measurement.kind = MEASUREMENT_KIND_KEEPALIVE;
    memcpy(measurement.data_keepalive.mac, device_mac_get_8(), sizeof(measurement.data_keepalive.mac));
    measurement.data_keepalive.counter = 0;

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000)); 
        measurement.data_keepalive.uptime_seconds = esp_timer_get_time() / (int64_t)1E6;
        measurement_put_fn(&measurement, false);
        measurement.data_keepalive.counter++;
    }
}


esp_err_t sensors_keepalive_start(output_enqueue_measurement_fn_t measurement_put_fn)
{
    return xTaskCreate(keepalive_task, "keepalive", 3072, (void *)measurement_put_fn, 5, NULL) == pdPASS ? ESP_OK : ESP_FAIL;
}
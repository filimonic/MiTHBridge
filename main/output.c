#include <strings.h>
#include <string.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "measurement.h"
#include "output.h"
#include "led_flasher.h"
#include "app_desc.h"

static const char *COLLECTD_PUTVAL_STRING_PREFIX_FMT = "%sPUTVAL %s_%02x%02x%02x%02x%02x%02x%02x%02x/%s-%02x%02x%02x%02x%02x%02x%02x%02x/%s N:"; // Should consume CHAR* as bridge device type, 8 bytes as bridge device mac, CHAR* as sensor device type, 8 bytes as sensor device mac, CHAR* as characteristic name (temperature, humidity, etc)

static QueueHandle_t output_queue = NULL;

int sprintf_collectd_value(output_print_fn_t print_fn, uint8_t is_duplicate, uint8_t bridge_mac[6], const char *sensor_device_type, uint8_t sensor_device_mac[6], const char *sensor_char_name, const char *sensor_char_fmt, ...)
{
    if (print_fn == NULL || COLLECTD_PUTVAL_STRING_PREFIX_FMT == NULL || sensor_device_type == NULL || sensor_char_name == NULL || sensor_char_fmt == NULL)
    {
        return -10000;
    }

    #if CONFIG_MITHBRIDGE_OUTPUT_DUPLICATES
        // Duplicates will be output
    #else 
        if (is_duplicate)
        {
            return 0;
        }
    #endif

    va_list args;
    char buf[256];
    int used_bytes = 0;
    int expected_bytes;
    int available_bytes;

    available_bytes = sizeof(buf) - used_bytes;
    expected_bytes = snprintf(buf + used_bytes, available_bytes, COLLECTD_PUTVAL_STRING_PREFIX_FMT,
                              is_duplicate ? "# DUP: " : "",
                              app_desc_name_get(),
                              bridge_mac[0], bridge_mac[1], bridge_mac[2], bridge_mac[3], bridge_mac[4], bridge_mac[5], bridge_mac[6], bridge_mac[7],
                              sensor_device_type,
                              sensor_device_mac[0], sensor_device_mac[1], sensor_device_mac[2], sensor_device_mac[3], sensor_device_mac[4], sensor_device_mac[5], sensor_device_mac[6], sensor_device_mac[7],
                              sensor_char_name);
    if (expected_bytes > available_bytes)
    {
        return -10001;
    }
    used_bytes += expected_bytes;
    available_bytes -= expected_bytes;

    va_start(args, sensor_char_fmt);
    expected_bytes = vsnprintf(buf + used_bytes, available_bytes, sensor_char_fmt, args);
    va_end(args);

    if (expected_bytes > available_bytes)
    {
        return -10002;
    }
    used_bytes += expected_bytes;
    available_bytes -= expected_bytes;

    return print_fn(buf);
}

void outputTask(void *pvParameters)
{
    output_print_fn_t print_fn = (output_print_fn_t)pvParameters;
    output_measurement_ex_t omex;
    const char *BRIDGE_SELF = "bridge_self";
    const char *SENSOR_BLE = "sensor_ble";
    //char KEEPALIVE_MESSAGE[128];
    //char *ABOUT_MESSAGE = NULL;
    //asprintf(&ABOUT_MESSAGE, "#\n# %s\n#    short name: %s\n#    version:    %s\n#    build date: %s %s\n#    homepage:   %s\n#", PROJECT_NAME_FULL, PROJECT_NAME_SHORT, app_desc_get()->version, app_desc_get()->date, app_desc_get()->time, PROJECT_HOMEPAGE );
    while (1)
    {
        if (xQueueReceive(output_queue, &omex, portMAX_DELAY) == pdPASS)
        {
            switch (omex.measurement.pvvx_data.kind)
            {
            case MEASUREMENT_KIND_PVVX:
            {
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.pvvx_data.bridge_mac, SENSOR_BLE, omex.measurement.pvvx_data.mac, "temperature", "%.2f", (float)omex.measurement.pvvx_data.temperature_c * 0.01f);
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.pvvx_data.bridge_mac, SENSOR_BLE, omex.measurement.pvvx_data.mac, "humidity", "%.2f", (float)omex.measurement.pvvx_data.humidity * 0.01f);
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.pvvx_data.bridge_mac, SENSOR_BLE, omex.measurement.pvvx_data.mac, "voltage-battery", "%.3f" , omex.measurement.pvvx_data.battery_mv * 0.001f);
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.pvvx_data.bridge_mac, SENSOR_BLE, omex.measurement.pvvx_data.mac, "percent-battery", "%hhu" , omex.measurement.pvvx_data.battery_level);
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.pvvx_data.bridge_mac, SENSOR_BLE, omex.measurement.pvvx_data.mac, "signal_power", "%hhd", omex.measurement.pvvx_data.rssi);
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.pvvx_data.bridge_mac, SENSOR_BLE, omex.measurement.pvvx_data.mac, "bool-isCodedPhy", "%hhu", omex.measurement.pvvx_data.is_phy_coded > 0 ? 1 : 0);
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.pvvx_data.bridge_mac, SENSOR_BLE, omex.measurement.pvvx_data.mac, "operations", "%hhu", omex.measurement.pvvx_data.counter);
                break;
            }
            case MEASUREMENT_KIND_ESP:
            {
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.data_esp.mac, BRIDGE_SELF, omex.measurement.data_esp.mac, "temperature", "%.2f", omex.measurement.data_esp.temperature_c);
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.data_esp.mac, BRIDGE_SELF, omex.measurement.data_esp.mac, "uptime", "%" PRIu32, omex.measurement.data_esp.uptime_seconds);
                sprintf_collectd_value(print_fn, omex.is_duplicate, omex.measurement.data_esp.mac, BRIDGE_SELF, omex.measurement.data_esp.mac, "operations", "%" PRIu8, omex.measurement.data_esp.counter);

                //Print project info
                // print_fn(ABOUT_MESSAGE);
                break;
            }
            default:
                break;
            }
        }
    }
}

esp_err_t output_start(output_print_fn_t print_fn)
{
    if (output_queue != NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }
    output_queue = xQueueCreate(MITHBRIDGE_OUTPUT_QUEUE_SIZE, sizeof(output_measurement_ex_t));
    if (output_queue == NULL)
    {
        return ESP_ERR_NO_MEM;
    }
    return xTaskCreate(outputTask, "output", 4096, (void *)print_fn, 1, NULL) == pdPASS ? ESP_OK : ESP_FAIL;
}

esp_err_t output_enqueue_measurement(measurement_t *measurement, bool is_duplicate)
{
    output_measurement_ex_t omex;
    omex.is_duplicate = is_duplicate;
    memcpy(&omex.measurement, measurement, sizeof(measurement_t));
    if (omex.measurement.kind == MEASUREMENT_KIND_PVVX)
    {
        led_flasher_register_event(is_duplicate);
    }
    return xQueueSend(output_queue, &omex, 0) == pdTRUE ? ESP_OK : ESP_FAIL;
}
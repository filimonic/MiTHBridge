#include "measurement.h"
#include <stddef.h>
#include <string.h>

int measurement_create_from_pvvx(measurement_t *measurement, uint8_t bridge_mac[8], uint8_t mac[8], int16_t temperature, uint16_t humidity, uint16_t battery_mv, uint8_t battery_level, uint8_t counter, char *device_name, uint8_t device_name_len, int8_t rssi, uint8_t is_phy_coded)
{
    if (measurement == NULL)
    {
        return -1;
    }

    measurement->kind = MEASUREMENT_KIND_PVVX;
    memcpy(measurement->pvvx_data.bridge_mac, bridge_mac, 8);
    memcpy(measurement->pvvx_data.mac, mac, 8);
    measurement->pvvx_data.rssi = rssi;
    measurement->pvvx_data.is_phy_coded = is_phy_coded > 0 ? 1 : 0;
    measurement->pvvx_data.temperature_c = temperature;
    measurement->pvvx_data.battery_level = battery_level;
    measurement->pvvx_data.battery_mv = battery_mv;
    measurement->pvvx_data.humidity = humidity;
    measurement->pvvx_data.counter = counter;
    if (device_name != NULL && device_name_len > 0)
    {
        measurement->pvvx_data.device_name_len = (device_name_len > sizeof(measurement->pvvx_data.device_name)) ? sizeof(measurement->pvvx_data.device_name) : device_name_len;
        memcpy(measurement->pvvx_data.device_name, device_name, measurement->pvvx_data.device_name_len);
    }
    else
    {
        measurement->pvvx_data.device_name_len = 0;
        measurement->pvvx_data.device_name[0] = '\0';
    }
    return 0;
}

int measurement_create_from_esp(measurement_t *measurement, uint8_t mac[8], uint32_t uptime_seconds, float temperature_c)
{
    if (measurement == NULL)
    {
        return -1;
    }
    measurement->kind = MEASUREMENT_KIND_ESP;
    memcpy(measurement->data_esp.mac, mac, 8);
    measurement->data_esp.uptime_seconds = uptime_seconds;
    measurement->data_esp.temperature_c = temperature_c;
    return 0;
}
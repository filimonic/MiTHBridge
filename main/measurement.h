#pragma once
#include "sdkconfig.h"
#include <stdint.h>

#define MEASUREMENT_KIND_PVVX (1)
#define MEASUREMENT_KIND_ESP (2)
#define MEASUREMENT_KIND_RESERVED_1 (3) // Was keepalive before 1.0.3.0

typedef uint8_t measurement_kind_t;

typedef union
{
    measurement_kind_t kind; // MEASUREMENT_INFO_KIND_*

    struct __attribute__((packed)) _measurement_data_pvvx
    {
        measurement_kind_t kind; // MEASUREMENT_INFO_KIND_*
        uint8_t counter;         // measurement count
        uint8_t bridge_mac[8];
        uint8_t mac[8];
        int16_t temperature_c; // x 0.01 degree
        uint16_t humidity;     // x 0.01 %
        uint16_t battery_mv;   // mV
        uint8_t battery_level; // 0..100 %

        char device_name[32];
        uint8_t device_name_len;
        int8_t rssi;
        int8_t is_phy_coded;
    } pvvx_data;

    struct __attribute__((packed)) _measurement_data_esp
    {
        measurement_kind_t kind; // MEASUREMENT_INFO_KIND_*
        uint8_t counter;         // measurement count
        uint8_t mac[8];
        uint32_t uptime_seconds;
        float temperature_c;
    } data_esp;

} measurement_t;

int measurement_create_from_pvvx(measurement_t *measurement, uint8_t bridge_mac[8], uint8_t mac[8], int16_t temperature, uint16_t humidity, uint16_t battery_mv, uint8_t battery_level, uint8_t counter, char *device_name, uint8_t device_name_len, int8_t rssi, uint8_t is_phy_coded);
int measurement_create_from_esp(measurement_t *measurement, uint8_t mac[8], uint32_t uptime_seconds, float temperature_c);
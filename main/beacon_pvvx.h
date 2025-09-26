#pragma once
#include "sdkconfig.h"
#include <stdint.h>

#define BEACON_PVVX_ADV_SERVICE_DATA ((uint8_t)0x16)
#define BEACON_PVVX_ADV_UUID ((uint16_t)0x181A)

typedef struct __attribute__((packed)) _beacon_pvvx
{
	uint8_t size;		   // = 18
	uint8_t uid;		   // = 0x16, 16-bit UUID
	uint16_t uuid;		   // = 0x181A, GATT Service 0x181A Environmental Sensing
	uint8_t mac[6];		   // [0] - lo, .. [] - hi digits
	int16_t temperature;   // x 0.01 degree
	uint16_t humidity;	   // x 0.01 %
	uint16_t battery_mv;   // mV
	uint8_t battery_level; // 0..100 %
	uint8_t counter;	   // measurement count
	uint8_t flags;
} beacon_pvvx_t;

int beacon_pvvx_parse(uint8_t *const adv_data, uint8_t adv_data_len, beacon_pvvx_t **pvvx_data, char **device_name, uint8_t *device_name_len);
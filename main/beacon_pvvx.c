#include "beacon_pvvx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define BEACON_PVVX_PARSE_ADV_ERR ((int8_t)-1)
#define BEACON_PVVX_PARSE_ADV_OK ((int8_t)0)
#define BEACON_PVVX_PARSE_ADV_ERR_NOT_PVVX__DATA_TOO_SHORT ((int8_t)(BEACON_PVVX_PARSE_ADV_OK + 1))
#define BEACON_PVVX_PARSE_ADV_ERR_NOT_PVVX__NO_SERVICE_DATA ((int8_t)(BEACON_PVVX_PARSE_ADV_OK + 2))
#define BEACON_PVVX_PARSE_ADV_ERR_NOT_PVVX__MALFORMED_FIELD_LENGTH ((int8_t)(BEACON_PVVX_PARSE_ADV_OK + 3))

int beacon_pvvx_parse(uint8_t *const adv_data, uint8_t adv_data_len, beacon_pvvx_t **pvvx_data, char **device_name, uint8_t *device_name_len)
{
	if (adv_data == NULL || pvvx_data == NULL || adv_data_len == 0)
	{
		return BEACON_PVVX_PARSE_ADV_ERR; // Error: NULL pointer or zero length
	}
	uint8_t *p_data = adv_data;
	uint8_t p_data_len = adv_data_len;
	bool found_service_data = false;

	while (p_data_len > 0)
	{
		uint8_t field_len = p_data[0];
		if (field_len == 0 || field_len + 1 > p_data_len)
		{
			return BEACON_PVVX_PARSE_ADV_ERR_NOT_PVVX__MALFORMED_FIELD_LENGTH; // Error: Malformed field length
		}
		uint8_t field_type = p_data[1];

		if (field_type == 0x16 && field_len == (sizeof(*(*pvvx_data)) - sizeof(field_len))) // Service Data - 16-bit UUID
		{
			if (((beacon_pvvx_t *)p_data)->uuid == BEACON_PVVX_ADV_UUID)
			{
				*pvvx_data = (beacon_pvvx_t *)p_data;
				found_service_data = true;
			}
		}
		else if (field_type == 0x09 || field_type == 0x08) // Complete Local Name
		{
			if (device_name != NULL && device_name_len != NULL)
			{
				*device_name = NULL;
				*device_name_len = 0;
			}
			*device_name_len = field_len - 1;
			*device_name = (char *)(p_data + 2);
		}

		p_data += field_len + 1;
		p_data_len -= field_len + 1;
	}

	if (!found_service_data)
	{
		return BEACON_PVVX_PARSE_ADV_ERR_NOT_PVVX__NO_SERVICE_DATA; // Error: No PVVX Service Data found
	}
	return BEACON_PVVX_PARSE_ADV_OK;
}
#include "device_mac.h"
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include "esp_err.h"
#include "esp_mac.h"



static uint8_t mac_addr_bytes[8];
static uint8_t * mac_addr_ptr = NULL;
    
uint8_t * device_mac_get_8()
{
    if (mac_addr_ptr == NULL)
    {
        memset(mac_addr_bytes, 0, sizeof(mac_addr_bytes));
        ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac_addr_bytes));
        mac_addr_ptr = mac_addr_bytes;
    }
    return mac_addr_ptr;
}
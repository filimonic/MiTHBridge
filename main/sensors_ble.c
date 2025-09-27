#include "sensors_ble.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "freertos/queue.h"
#include "freertos/queue.h"
#include "beacon_pvvx.h"
#include "measurement.h"
#include "device_mac.h"

output_enqueue_measurement_fn_t measurement_put_fn = NULL;

typedef struct __attribute__((packed)) _sensor_ble_cache_item
{
    int64_t timestamp;
    uint8_t mac[6];
    uint8_t counter;
} sensor_ble_cache_item_t;

sensor_ble_cache_item_t cache[SENSOR_BLE_MEASUREMENT_CACHE_SIZE];

void sensor_ble_measurement_cache_init(void)
{
    memset(cache, 0, sizeof(cache));
}

bool sensor_ble_measurement_cache_is_duplicate(uint8_t mac[6], uint8_t counter)
{
    int64_t now = esp_timer_get_time();
    // Check for existing entry
    int oldest_entry_index = 0;
    int64_t oldest_entry_timestamp = 0;
    for (int i = 0; i < SENSOR_BLE_MEASUREMENT_CACHE_SIZE; i++)
    {
        if (cache[i].timestamp == 0)
        {
            oldest_entry_index = i;
            break; // Found empty slot
        }
        if (cache[i].timestamp < oldest_entry_timestamp)
        {
            oldest_entry_index = i;
            oldest_entry_timestamp = cache[i].timestamp;
        }
        if (memcmp(cache[i].mac, mac, 6) == 0)
        {
            // Found existing entry
            if (cache[i].counter == counter)
            {
                if ((now - cache[i].timestamp) >= SENSOR_BLE_MEASUREMENT_CACHE_EXPIRY_MICROS)
                {
                    // Expired entry, update timestamp and counter
                    cache[i].timestamp = now;
                    cache[i].counter = counter;
                    return false;
                }
                return true; // Duplicate found
            }
            else
            {
                // Update existing entry
                cache[i].counter = counter;
                cache[i].timestamp = now;
                return false;
            }
        }
    }
    // Add new entry

    cache[oldest_entry_index].counter = counter;
    cache[oldest_entry_index].timestamp = now;
    memcpy(cache[oldest_entry_index].mac, mac, 6);

    return false;
}

static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_EXT_ADV_REPORT_EVT:
    {
        // ESP_LOGI(TAG, "ESP_GAP_BLE_EXT_ADV_REPORT_EVT");
        esp_ble_gap_cb_param_t *cb_param = param;
        esp_ble_gap_ext_adv_report_t *adv_report = &cb_param->ext_adv_report.params;

        if (adv_report->data_status != ESP_BLE_GAP_EXT_ADV_DATA_COMPLETE)
        {
            break;
        }
        beacon_pvvx_t *pvvx_data = NULL;
        char *device_name = NULL;
        uint8_t device_name_len = 0;
        if (beacon_pvvx_parse(adv_report->adv_data, adv_report->adv_data_len, &pvvx_data, &device_name, (uint8_t *)&device_name_len) == 0)
        {
            uint8_t mac_8[8] = {0,0,0,0,0,0,0,0};
            memcpy(mac_8, pvvx_data->mac, sizeof(pvvx_data->mac));
            measurement_t measurement;
            if (measurement_create_from_pvvx(&measurement,
                                             device_mac_get_8(),
                                             (uint8_t *)mac_8,
                                             pvvx_data->temperature,
                                             pvvx_data->humidity,
                                             pvvx_data->battery_mv,
                                             pvvx_data->battery_level,
                                             pvvx_data->counter,
                                             device_name,
                                             device_name_len,
                                             adv_report->rssi,
                                             adv_report->primary_phy == ESP_BLE_GAP_PRI_PHY_CODED ? 1 : 0) == 0)
            {
                measurement_put_fn(&measurement, sensor_ble_measurement_cache_is_duplicate((uint8_t *)pvvx_data->mac, pvvx_data->counter));
            }
        };
        break;
    }
    default:
        break;
    }
}

esp_err_t sensor_ble_start(output_enqueue_measurement_fn_t fn)
{
    sensor_ble_measurement_cache_init();
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_cb));
    esp_ble_ext_scan_cfg_t scan_cfg = {
        .scan_type = BLE_SCAN_TYPE_PASSIVE,
        .scan_interval = 0x4000,
        .scan_window = 0x4000,
    };
    esp_ble_ext_scan_params_t ext_scan_params = {
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
        .cfg_mask = ESP_BLE_GAP_EXT_SCAN_CFG_CODE_MASK | ESP_BLE_GAP_EXT_SCAN_CFG_UNCODE_MASK,
        .coded_cfg = scan_cfg,
        .uncoded_cfg = scan_cfg,
    };
    ESP_ERROR_CHECK(esp_ble_gap_set_ext_scan_params(&ext_scan_params));
    measurement_put_fn = fn;
    ESP_ERROR_CHECK(esp_ble_gap_start_ext_scan(0, 0));

    return ESP_OK;
}

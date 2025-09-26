#include "setup_gpio.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_ENABLE_GPIO (3)
#define XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_ENABLE_LEVEL (0)
#define XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_ENABLE_DELAY (100)
#define XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_EXT_ANT_GPIO (14)
#define XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_EXT_ANT_LEVEL (1)

esp_err_t setup_gpio()
{
    #if MITHBRIDGE_USE_GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    #if CONFIG_LED_FLASHER_ENABLED
        io_conf.pin_bit_mask |=  (1ULL << CONFIG_LED_FLASHER_LED_GPIO);
    #endif

    #if CONFIG_MITHBRIDGE_BOARD_USE_EXTERNAL_ANT
        #if defined(CONFIG_MITHBRIDGE_BOARD_ESP32C6_XIAO)
            // On XIAO ESP32C6, 
            // GPIO3 enables RF Switch
            // GPIO14 enables external antenna
            io_conf.pin_bit_mask |=  (1ULL << XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_ENABLE_GPIO);
            io_conf.pin_bit_mask |=  (1ULL << XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_EXT_ANT_GPIO);
        #endif
    #endif
    
    
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    #if CONFIG_LED_FLASHER_ENABLED 
    ESP_ERROR_CHECK(gpio_set_level(CONFIG_LED_FLASHER_LED_GPIO, (CONFIG_LED_FLASHER_INVERTED ? 1 : 0))); // Turn off LED if inverted
    #endif

    #if CONFIG_MITHBRIDGE_BOARD_USE_EXTERNAL_ANT
        #if defined(CONFIG_MITHBRIDGE_BOARD_ESP32C6_XIAO)
            vTaskDelay(pdMS_TO_TICKS(XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_ENABLE_DELAY));
            ESP_ERROR_CHECK(gpio_set_level(XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_ENABLE_GPIO, XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_ENABLE_LEVEL)); // Enable RF Switch
            vTaskDelay(pdMS_TO_TICKS(XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_ENABLE_DELAY));
            ESP_ERROR_CHECK(gpio_set_level(XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_EXT_ANT_GPIO, XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_EXT_ANT_LEVEL)); // Enable RF Switch
            vTaskDelay(pdMS_TO_TICKS(XIAO_ESP32C6_EXT_ANTENNA_RFSWITCH_ENABLE_DELAY));
        #endif
    #endif

    #endif
    return ESP_OK;
}
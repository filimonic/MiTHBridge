#include "led_flasher.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
// static const char *TAG = "led_flasher";

#define LED_FLASHER_LED_ON (CONFIG_LED_FLASHER_INVERTED ? 0 : 1)
#define LED_FLASHER_LED_OFF (CONFIG_LED_FLASHER_INVERTED ? 1 : 0)

QueueHandle_t led_event_queue = NULL;

#if CONFIG_LED_FLASHER_ENABLED
void led_flash_task(void *arg)
{
    uint8_t is_duplicate;
    while (1)
    {
        if (xQueueReceive(led_event_queue, &is_duplicate, portMAX_DELAY) == pdPASS)
        {
            gpio_set_level(CONFIG_LED_FLASHER_LED_GPIO, LED_FLASHER_LED_ON);
            if (is_duplicate)
            {
                
                vTaskDelay(pdMS_TO_TICKS(CONFIG_LED_FLASHER_ON_INTERVAL_MS / 5));
                gpio_set_level(CONFIG_LED_FLASHER_LED_GPIO, LED_FLASHER_LED_OFF);
                vTaskDelay(pdMS_TO_TICKS(CONFIG_LED_FLASHER_ON_INTERVAL_MS / 5));
                gpio_set_level(CONFIG_LED_FLASHER_LED_GPIO, LED_FLASHER_LED_ON);
                vTaskDelay(pdMS_TO_TICKS(CONFIG_LED_FLASHER_ON_INTERVAL_MS / 5));
                gpio_set_level(CONFIG_LED_FLASHER_LED_GPIO, LED_FLASHER_LED_OFF);
                vTaskDelay(pdMS_TO_TICKS(CONFIG_LED_FLASHER_ON_INTERVAL_MS / 5));
                gpio_set_level(CONFIG_LED_FLASHER_LED_GPIO, LED_FLASHER_LED_ON);
                vTaskDelay(pdMS_TO_TICKS(CONFIG_LED_FLASHER_ON_INTERVAL_MS / 5));
            }
            else 
            {
                vTaskDelay(pdMS_TO_TICKS(CONFIG_LED_FLASHER_ON_INTERVAL_MS));
            }
            
            gpio_set_level(CONFIG_LED_FLASHER_LED_GPIO, LED_FLASHER_LED_OFF);
            vTaskDelay(pdMS_TO_TICKS(CONFIG_LED_FLASHER_OFF_INTERVAL_MS));
        }
    }
}
#endif
esp_err_t led_flasher_start(void)
{
#if CONFIG_LED_FLASHER_ENABLED
    led_event_queue = xQueueCreate(CONFIG_LED_FLASHER_QUEUE_SIZE, sizeof(uint8_t));
    if (led_event_queue == NULL)
    {
        return ESP_ERR_NO_MEM;
    }
    return xTaskCreate(led_flash_task, "led_flash_task", 2048, NULL, 5, NULL) == pdPASS ? ESP_OK : ESP_FAIL;
#else
    return ESP_OK;
#endif
}

esp_err_t led_flasher_register_event(bool is_duplicate)
{
#if CONFIG_LED_FLASHER_ENABLED

    if (is_duplicate && !CONFIG_LED_FLASHER_DUPLICATES_ENABLED)
    {
        return ESP_OK;
    }
    uint8_t value = is_duplicate ? 1 : 0;
    return (xQueueSend(led_event_queue, &value, 0) == pdTRUE ? ESP_OK : ESP_FAIL);
#else
    return ESP_OK
#endif
}
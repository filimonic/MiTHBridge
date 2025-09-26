#pragma once
#include "sdkconfig.h"
#include "esp_err.h"

#define MITHBRIDGE_USE_GPIO CONFIG_LED_FLASHER_ENABLED || CONFIG_MITHBRIDGE_BOARD_USE_EXTERNAL_ANT

esp_err_t setup_gpio();

#pragma once
#include "sdkconfig.h"
#include "esp_err.h"
#include "output.h"

typedef void (*watchdog_ble_sesors_feed_fn_t)(void *);


esp_err_t watchdog_ble_sensors_start();
void watchdog_ble_sensors_feed();



#pragma once
#include "sdkconfig.h"
#include "esp_err.h"
#include "output.h"

#define SENSORS_SELF_INTERVAL_MS (CONFIG_SENSORS_SELF_INTERVAL_SECONDS * 1000)

esp_err_t sensors_keepalive_start(output_enqueue_measurement_fn_t fn);

#pragma once
#include "sdkconfig.h"
#include <stdbool.h>
#include "esp_err.h"
#include "measurement.h"

#ifdef CONFIG_MITHBRIDGE_OUTPUT_QUEUE_SIZE
#if CONFIG_MITHBRIDGE_OUTPUT_QUEUE_SIZE < 10
#define MITHBRIDGE_OUTPUT_QUEUE_SIZE 10
#else
#define MITHBRIDGE_OUTPUT_QUEUE_SIZE CONFIG_MITHBRIDGE_OUTPUT_QUEUE_SIZE
#endif
#else
#define MITHBRIDGE_OUTPUT_QUEUE_SIZE 10
#endif

typedef int (*output_print_fn_t)(void *);
typedef esp_err_t (*output_enqueue_measurement_fn_t)(measurement_t *, bool);

typedef struct _output_measurement_ex
{
    uint8_t is_duplicate;
    measurement_t measurement;
} output_measurement_ex_t;


esp_err_t output_start(output_print_fn_t print_fn);
esp_err_t output_enqueue_measurement(measurement_t *measurement, bool is_duplicate);
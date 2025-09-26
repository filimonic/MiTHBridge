#pragma once
#include "sdkconfig.h"
#include "esp_app_desc.h"

#define PROJECT_NAME_SHORT (app_desc_get()->project_name)
#define PROJECT_NAME_FULL "Mi Thermometer & Hygrometer Bridge"
#define PROJECT_HOMEPAGE "https://github.com/filimonic/MiTHBridge"

const char *app_desc_name_get();
const esp_app_desc_t *app_desc_get();

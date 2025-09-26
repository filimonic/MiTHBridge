#include "sdkconfig.h"
#include "esp_app_desc.h"

static const esp_app_desc_t *app_desc = NULL;

const esp_app_desc_t *app_desc_get();

const char *app_desc_name_get()
{
    return app_desc_get()->project_name;
}

const esp_app_desc_t *app_desc_get() {
    if (app_desc == NULL)
    {
        app_desc = esp_app_get_description();
    }
    return app_desc;
}
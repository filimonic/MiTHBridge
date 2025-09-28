set(srcs 
    "app_desc.c"
    "beacon_pvvx.c"
    "device_mac.c"
    "led_flasher.c"
    "main.c"
    "measurement.c"
    "output.c"
    "sensors_ble.c"
    "sensors_self.c"
    "setup_gpio.c"
    "watchdog_ble_sensors.c")

set(preqs
    "bt"
    "nvs_flash"
    "esp_timer"
    "driver"
    "esp_driver_tsens"
    "esp_app_format"
)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES "${preqs}")

# Add Kconfig for menuconfig
set(COMPONENT_KCONFIG main/Kconfig)

set(srcs  "main.c" "beacon_pvvx.c" "led_flasher.c" "sensors_self.c" "sensors_ble.c" "measurement.c" "output.c" "app_desc.c" "setup_gpio.c" "device_mac.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES bt nvs_flash esp_timer driver esp_driver_tsens esp_app_format)

# Add Kconfig for menuconfig
set(COMPONENT_KCONFIG main/Kconfig)

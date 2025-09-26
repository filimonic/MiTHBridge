#pragma once
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32C6
    #if !CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_80
        #error "Please set ESP to 80 MHz (Component config -> ESP System settings -> CPU frequency)"
    #endif

    #if !CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
        #error "Please set ESP to output to USB/JTAG port (Component config -> ESP System Settings -> Channel for console output)"
    #endif

    #if !CONFIG_USJ_ENABLE_USB_SERIAL_JTAG
        #error "Please enable ESP USB Serial/JTAG (Component config -> ESP-Driver:USB Serial/JTAG Configuration -> Enable USB-Serial-JTAG Module)"
    #endif


#endif
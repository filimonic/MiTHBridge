#!/bin/sh

# Configuration variables
DEVICE=""  # Leave empty for auto-detection
VENDOR_PRODUCT="303a:1001"
BAUDRATE=115200
RETRY_DELAY=5
STTY_TIMEOUT=10
READ_TIMEOUT=60
DEBUG_SYSLOG=${DEBUG_SYSLOG:-${DEBUG:-0}}
DEBUG_STDERR=${DEBUG_STDERR:-${DEBUG:-0}}
LOG_TAG=$(basename "$0")

# Function to find the correct ttyACM device
find_espressif_device() {
    for uevent_file in /sys/class/tty/ttyACM*/device/uevent; do
        [ -f "$uevent_file" ] || continue
        if grep -q "^PRODUCT=${VENDOR_PRODUCT//:/\\/}" "$uevent_file"; then
            tty_name=$(echo "$uevent_file" | cut -d'/' -f5)
            [ -e "/dev/$tty_name" ] && echo "/dev/$tty_name" && return 0
        fi
    done
    return 1
}

# Function to reset USB device
reset_usb_device() {
    error_echo "# Resetting USB device $VENDOR_PRODUCT"
    if command -v usbreset >/dev/null 2>&1; then
        usbreset "$VENDOR_PRODUCT" >/dev/null 2>&1 && return 0
    else
        error_echo "# usbreset command not found"
    fi
    return 1
}

# Function to print debug messages
debug_echo() {
    [ "$DEBUG_STDERR" -eq 1 ] && echo "$1" >&2
    [ "$DEBUG_SYSLOG" -eq 1 ] && logger -t $LOG_TAG "$1"
}

error_echo() {
    logger -s -t $LOG_TAG -p err "$1"
}

# Main execution
debug_echo "# Starting serial monitor for Espressif USB JTAG/serial debug unit"
debug_echo "# DEBUG_SYSLOG mode is $( [ "$DEBUG_SYSLOG" -eq 1 ] && echo "enabled" || echo "disabled" )"
debug_echo "# DEBUG_STDERR mode is $( [ "$DEBUG_STDERR" -eq 1 ] && echo "enabled" || echo "disabled" )"

while true; do
    # Auto-detect device if not specified or invalid
    if [ -z "$DEVICE" ] || [ ! -c "$DEVICE" ]; then
        debug_echo "# Looking for Espressif device..."
        detected_device=$(find_espressif_device)
        if [ -n "$detected_device" ]; then
            DEVICE="$detected_device"
            debug_echo "# Using device: $DEVICE"
        else
            error_echo "# No Espressif device found, resetting USB and retrying in $RETRY_DELAY seconds..."
            reset_usb_device
            sleep "$RETRY_DELAY"
            continue
        fi
    fi
    
    # Verify device type using uevent
    tty_name=$(basename "$DEVICE")
    uevent_file="/sys/class/tty/$tty_name/device/uevent"
    if [ ! -f "$uevent_file" ] || ! grep -q "^PRODUCT=${VENDOR_PRODUCT//:/\\/}" "$uevent_file"; then
        error_echo "# Device validation failed, resetting USB and re-detecting..."
        reset_usb_device
        DEVICE=""
        sleep "$RETRY_DELAY"
        continue
    fi
    
    # Set serial parameters
    debug_echo "# Configuring $DEVICE at $BAUDRATE baud"
    if ! timeout "$STTY_TIMEOUT" stty -F "$DEVICE" "$BAUDRATE"; then
        debug_echo "# stty command failed, resetting USB and re-detecting device..."
        reset_usb_device
        DEVICE=""
        sleep "$RETRY_DELAY"
    fi
    
    # Read from serial port
    debug_echo "# Reading from $DEVICE"
    while IFS= read -r -t "$READ_TIMEOUT" line; do
        [ "${line}" = "" ] && continue # skip empty lines
        debug_echo "INCOMING: '$line'"
        echo "$line" | grep "^PUTVAL" # Always print PUTVAL lines
        # Other lines are silently dropped
    done < "$DEVICE"
    
    # If we get here, the read timed out
    error_echo "# Timeout: No data received from $DEVICE for $READ_TIMEOUT seconds"
    error_echo "# Resetting USB device before exit"
    reset_usb_device
    error_echo "# Exiting script"
    exit 1
done

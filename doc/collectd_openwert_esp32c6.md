## Connecting ESP32-C6 to OpenWRT (Native ESP JTAG/Serial) with CollectD

Note: I skip CollectD general configuration here. Only things related to reading data from MiTHBridge will be noted.

### HUB problems: 

**Warning**: I had problems with some USB hubs while working with ESP32-C6 with OpenWrt. 
Those problem symptoms are: `cat < /dev/ttyACM0` shows nothing, and after few retries, there are such messages in `dmesg`: 
```shell
# dmesg
[  313.482359] xhci-hcd xhci-hcd.8.auto: xHCI host not responding to stop endpoint command
[  313.503287] xhci-hcd xhci-hcd.8.auto: xHCI host controller not responding, assume dead
[  313.504103] xhci-hcd xhci-hcd.8.auto: HC died; cleaning up
[  313.505428] usb 2-1: USB disconnect, device number 2
[  313.505920] usb 2-1.1: USB disconnect, device number 4
```
I replaced the USB hub to BASEUS.

### Install and check that OpenWrt box can read sensors:

We will need aditional tools: 
* `kmod-usb-acm` module to support `cdc_acm` devices like ESP32-C6 native USB-Serial `303a:1001 Espressif USB JTAG/serial debug unit`
* `coreutils-stty` for `stty` binary to change baudrate of serial port
* `coreutils-timeout` because `stty` may hang
* `collectd` and `collectd-mod-exec`

Install them

```shell
# opkg update
# opkg install kmod-usb-acm coreutils-stty collectd-mod-exec 
```

Test it with

```shell
# logread | grep acm
Thu Sep 25 17:14:39 2025 kern.info kernel: [    8.505309] cdc_acm 2-1.2:1.0: ttyACM0: USB ACM device
Thu Sep 25 17:14:39 2025 kern.info kernel: [    8.505883] usbcore: registered new interface driver cdc_acm
Thu Sep 25 17:14:39 2025 kern.info kernel: [    8.506390] cdc_acm: USB Abstract Control Model driver for USB modems and ISDN adapters

# dmesg | grep acm
[    8.505309] cdc_acm 2-1.2:1.0: ttyACM0: USB ACM device
[    8.505883] usbcore: registered new interface driver cdc_acm
[    8.506390] cdc_acm: USB Abstract Control Model driver for USB modems and ISDN adapters
```

Check device exists: 

```shell
# ls  /dev/ttyACM*
/dev/ttyACM0
```

Set baudrate on serial port 

```shell
# stty -F /dev/ttyACM0 115200
```

And listen for messages, you should see something like this

```shell
# cat < /dev/ttyACM0
# KEEPALIVE 580
PUTVAL MiTHBridge_????????????????/sensor_ble-????????????0000/temperature N:16.03
PUTVAL MiTHBridge_????????????????/sensor_ble-????????????0000/humidity N:43.79
PUTVAL MiTHBridge_????????????????/sensor_ble-????????????0000/voltage-battery N:3.149
PUTVAL MiTHBridge_????????????????/sensor_ble-????????????0000/percent-battery N:100
PUTVAL MiTHBridge_????????????????/sensor_ble-????????????0000/signal_power N:-67
PUTVAL MiTHBridge_????????????????/sensor_ble-????????????0000/bool-isCodedPhy N:1
PUTVAL MiTHBridge_????????????????/sensor_ble-????????????0000/operations N:161
```

### Confugure collectd

Download [mithbridge.sh](../scripts/mithbridge.sh) to `/usr/lib/collectd/mithbridge.sh`

Save this script for sysupgrades: 

```shell
# echo  /usr/lib/collectd/mithbridge.sh >> /etc/sysupgrade.conf
```

Add configuration to `/etc/collectd.conf`: 

```xml
<Plugin "exec">
  Exec "daemon:dialout" "/bin/ash" "/usr/lib/collectd/mithbridge.sh"
</Plugin>
```

Restart CollectD

```shell
#/etc/init.d/collectd restart
```
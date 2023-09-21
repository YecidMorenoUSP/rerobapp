# Readme.md
by [Yecid Moreno](https://github.com/YecidMorenoUSP)

eMMC image : [AM335x Debian 11.7 2023-08-05 4GB eMMC IoT Flasher](https://www.beagleboard.org/distros/am335x-debian-11-7-2023-08-05-4gb-emmc-iot-flasher)
```bash
[username:pawword]: debian:temppwd
```

Open VsCode
Install [Remote - SSH]


## Connect to WIFI
wpa_cli
wpa_cli -i wlan0
scan
scan_result
add_network

set_network 1 ssid "rerob"
set_network 1 psk "rerob123"

enable_network 1

save_config

quit

sudo dhclient wlan0

## install packages
sudo apt update

sudo apt-get install gcc-arm-linux-gnueabihf
sudo apt-get install g++-arm-linux-gnueabihf

arm-linux-gnueabihf-g++ exotest.cpp  -o exo2
scp ./exo2 debian@192.168.7.2:/home/debian

## ssh witout password
ssh-keygen
ssh-copy-id debian@192.168.7.2
#!/bin/bash

sudo timedatectl set-timezone America/Sao_Paulo

sudo /sbin/ip link set can0 down

sleep 1

config-pin p9.19 can
config-pin p9.20 can

config-pin p9.24 can
config-pin p9.26 can 

sleep 1

# sudo /sbin/ip link set can0 up type can bitrate 500000  
sudo /sbin/ip link set can0 up type can bitrate 500000  
sudo ifconfig can0 txqueuelen 1000

sleep 1
sudo /sbin/ip link set can1 up type can bitrate 500000  
sudo ifconfig can1 txqueuelen 1000

# sudo su
# echo  990000 >   /proc/sys/kernel/sched_rt_runtime_us
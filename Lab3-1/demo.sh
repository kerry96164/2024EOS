#!/bin/sh

set -x #prints each command and its arguments to the terminal before executing it
#set -e #Exit immediately if a command exits with a non-zero status

rmmod -f lab3-1_driver
insmod lab3-1_driver.ko

./writer '312513028' /dev/LED_device

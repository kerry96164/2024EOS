#!/bin/sh

set -x #prints each command and its arguments to the terminal before executing it
#set -e #Exit immediately if a command exits with a non-zero status

sudo rmmod -f lab3-2_driver
sudo insmod lab3-2_driver.ko

./writer '312513028' /dev/LED7_device

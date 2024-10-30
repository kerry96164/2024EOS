#!/bin/sh

set -x #prints each command and its arguments to the terminal before executing it

sudo rmmod -f LED_7_seg_driver
sudo rmmod -f LED_Bar_Array_driver

sudo insmod LED_7_seg_driver.ko
sudo insmod LED_Bar_Array_driver.ko

./hw1
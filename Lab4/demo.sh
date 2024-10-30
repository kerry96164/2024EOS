#!/bin/sh

set -x #prints each command and its arguments to the terminal before executing it
#set -e #Exit immediately if a command exits with a non-zero status

sudo rmmod -f mydev
sudo insmod mydev.ko
sudo mknod /dev/mydev c 233 0

./writer 'Kerry' /dev/mydev & #run in subshell
./reader 192.168.127.1 8888 /dev/mydev

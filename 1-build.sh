#!/bin/bash
# Program:
#       This program build RPi kernel for NYCU 2024 EOS.
# Input:
#       $1 Linux main folder path
#       $2 config option (bcm2711_defconfig or menuconfig)
#       $3 Make how many jobs (commands) to run in parallel.
# History:
# 2024/09/23	Kerry	First release

set -e # exit on the first error.

[ "$#" -lt 3 ] && echo "Useage $./build.sh <path> <bcm2711_defconfig or menuconfig> <jobs number>" && exit 0
folder_PATH=$1
CONFIG=$2
N=$3

#read -p "Please input Linux main folder path: " PATH       # 提示使用者輸入

cd $folder_PATH
var_path=$PWD  
echo $var_path


KERNEL=kernel8
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- $CONFIG
make clean
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image modules dtbs -j$N

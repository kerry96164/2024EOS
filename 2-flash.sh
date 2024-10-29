#!/bin/bash
# Program:
#       This program flash RPi kernel to SD card for NYCU 2024 EOS.
# Input:
#       $1 Linux main folder path
#       $2 DEVICE Name
#       $3 Image Name
# History:
# 2024/09/23	Kerry	First release

set -e # exit on the first error.

[ "$#" -lt 3 ] && echo "Useage $./flash.sh <path> <DEVICE Name> <Image Name>" && exit 0
folder_PATH=$1
DEVICE=$2
Image_NAME=$3

cd $folder_PATH
var_path=$PWD  
echo $var_path

# ======Unmount & Clear======
sudo umount -v /dev/sdc1 || true
sudo umount -v /dec/sdc2 || true
rm -rf mnt
# ======Mount======
mkdir mnt
mkdir mnt/fat32
mkdir mnt/ext4
mount -v /dev/${DEVICE}1 mnt/fat32
mount -v /dev/${DEVICE}2 mnt/ext4
echo "Mount Success"

# ======Flash======
sudo env PATH=$PATH make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=mnt/ext4 modules_install

sudo cp arch/arm64/boot/Image mnt/fat32/kernel8-${Image_NAME}.img

sudo cp arch/arm64/boot/dts/broadcom/*.dtb mnt/fat32/
sudo cp arch/arm64/boot/dts/overlays/*.dtb* mnt/fat32/overlays/
sudo cp arch/arm64/boot/dts/overlays/README mnt/fat32/overlays/

# ======Unmount======
sudo umount -v mnt/fat32 
sudo umount -v mnt/ext4
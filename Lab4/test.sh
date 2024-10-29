set -x #prints each command and its arguments to the terminal before executing it
set -e #Exit immediately if a command exits with a non-zero status

make clean
make x86_build
sudo rmmod mydev.ko
sudo insmod mydev.ko

sudo mknod /dev/mydev c 233 0
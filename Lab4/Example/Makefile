#CC=gcc #compiler

obj-m:= hello_driver.o
HOST_KERNEL_VER=$(shell uname -r)
OUT_DIR := build
ARCH?=arm64 
CROSS_COMPILE?=aarch64-linux-gnu-

all: RPI_build
clean: x86_clean RPI_clean

x86_build:
	sudo make -C /lib/modules/$(HOST_KERNEL_VER)/build M=$(PWD) modules
x86_clean:
	sudo make -C /lib/modules/$(HOST_KERNEL_VER)/build M=$(PWD) clean

RPI_build:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ~/2024EOS/linux-bcm2711_defconfig M=$(PWD) modules
RPI_clean:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ~/2024EOS/linux-bcm2711_defconfig M=$(PWD) clean

.PHONY: all clean x86_build x86_clean RPI_build RPI_clean
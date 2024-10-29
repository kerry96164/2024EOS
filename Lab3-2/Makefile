#CC=gcc #compiler

LAB_NAME := lab3-2
obj-m:= $(LAB_NAME)_driver.o
HOST_KERNEL_VER=$(shell uname -r)
OUT_DIR := build
ARCH?=arm64 
CROSS_COMPILE?=aarch64-linux-gnu-
ccflags-y += -std=gnu99

all: RPI_build
clean: RPI_clean

RPI_build:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ~/2024EOS/linux-bcm2711_defconfig M=$(PWD) modules
	$(CROSS_COMPILE)gcc $(LAB_NAME)_writer.c -o writer
RPI_clean:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ~/2024EOS/linux-bcm2711_defconfig M=$(PWD) clean
	rm -f writer

.PHONY: all clean RPI_build RPI_clean zip

zip: all
	rm -rf $(LAB_NAME)
	rm -rf $(LAB_NAME).zip
	mkdir $(LAB_NAME)
	cp $(LAB_NAME)_driver.ko $(LAB_NAME)
	cp writer $(LAB_NAME)
	cp Makefile $(LAB_NAME)
	cp $(LAB_NAME)_driver.c $(LAB_NAME)
	cp $(LAB_NAME)_writer.c $(LAB_NAME)
	cp demo.sh $(LAB_NAME)
	zip -r $(LAB_NAME).zip $(LAB_NAME)
	rm -rf $(LAB_NAME)
	make clean
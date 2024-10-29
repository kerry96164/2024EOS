#CC=gcc #compiler

obj-m:= mydev.o
HOST_KERNEL_VER=$(shell uname -r)
OUT_DIR := build
ARCH?=arm64 
CROSS_COMPILE?=aarch64-linux-gnu-
#ccflags-y := -std=gnu99



all: RPI_build
clean: x86_clean RPI_clean

x86_build:
	sudo make -C /lib/modules/$(HOST_KERNEL_VER)/build M=$(PWD) modules
	gcc writer.c -o writer
	gcc reader.c -o reader
x86_clean:
	sudo make -C /lib/modules/$(HOST_KERNEL_VER)/build M=$(PWD) clean
	rm -f writer
	rm -f reader

RPI_build:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ~/2024EOS/linux-bcm2711_defconfig M=$(PWD) modules
	$(CROSS_COMPILE)gcc writer.c -o writer
	$(CROSS_COMPILE)gcc reader.c -o reader
RPI_clean:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ~/2024EOS/linux-bcm2711_defconfig M=$(PWD) clean
	rm -f writer
	rm -f reader

.PHONY: all clean x86_build x86_clean RPI_build RPI_clean zip

zip: all
	rm -rf Lab4
	rm -rf Lab4.zip
	mkdir Lab4
	cp mydev.ko Lab4
	cp writer Lab4
	cp reader Lab4
	cp Makefile Lab4
	cp mydev.c Lab4
	cp writer.c Lab4
	cp reader.c Lab4
	cp seg.py Lab4
	cp demo.sh Lab4
	zip -r Lab4.zip Lab4
	rm -rf Lab4
	make clean
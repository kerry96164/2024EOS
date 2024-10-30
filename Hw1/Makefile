ARCH?=arm64 
CROSS_COMPILE?=aarch64-linux-gnu-
obj-m += LED_Bar_Array_driver.o
obj-m += LED_7_seg_driver.o
NAME := Hw1

all: RPI_build
clean: RPI_clean

RPI_build:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ../linux-bcm2711_defconfig M=$(PWD) modules
	$(CROSS_COMPILE)gcc hw1.c -o hw1
RPI_clean:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ../linux-bcm2711_defconfig M=$(PWD) clean
	rm -f hw1


.PHONY: all clean RPI_build RPI_clean zip

zip: all
	rm -rf $(NAME)
	rm -rf $(NAME).zip
	mkdir $(NAME)
	cp LED_Bar_Array_driver.ko $(NAME)
	cp LED_7_seg_driver.ko $(NAME)
	cp hw1.c $(NAME)
	cp hw1 $(NAME)
	cp Makefile $(NAME)
	cp RUN.sh $(NAME)
	cp LED_Bar_Array_driver.c $(NAME)
	cp LED_7_seg_driver.c $(NAME)
	
	zip -r $(NAME).zip $(NAME)
	rm -rf $(NAME)
	make clean
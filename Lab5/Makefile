#CC=gcc #compiler

CROSS_COMPILE?=aarch64-linux-gnu-

all: x86_build
clean: x86_clean RPI_clean

x86_build:
	gcc lab5.c -o lab5
x86_clean:
	rm -f lab5

RPI_build:
	$(CROSS_COMPILE)gcc lab5.c -o writer
RPI_clean:
	rm -f lab5

.PHONY: all clean x86_build x86_clean RPI_build RPI_clean zip

zip: all
	rm -rf Lab5
	rm -rf Lab5.zip
	mkdir Lab5
	cp demo.sh Lab5
	cp lab5 Lab5
	zip -r Lab5.zip Lab5
	rm -rf Lab5
	make clean
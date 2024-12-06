ARCHARCH?=arm64 
CROSS_COMPILE?=aarch64-linux-gnu-
NAME := Hw3

all: x86_build
clean: x86_clean RPI_clean

RPI_build:
	$(CROSS_COMPILE)gcc hw3.c -o hw3
	$(CROSS_COMPILE)gcc hw3_client.c -o hw3_client
RPI_clean:
	rm -f hw3
	rm -f hw3_client

x86_build:
	gcc hw3.c -o hw3
	gcc hw3_client.c -o hw3_client
x86_clean:
	rm -f hw3
	rm -f hw3_client

.PHONY: all clean x86_build x86_clean RPI_build RPI_clean zip

zip: all
	rm -rf $(NAME)
	rm -rf $(NAME).zip
	mkdir $(NAME)
	cp hw3.c $(NAME)
	cp hw3 $(NAME)
	cp Makefile $(NAME)
	cp hw3_checker $(NAME)
	cp hw3_client.c $(NAME)
	cp hw3_client $(NAME)
	zip -r $(NAME).zip $(NAME)
	rm -rf $(NAME)
	make clean
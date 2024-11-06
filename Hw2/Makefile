ARCH?=arm64 
CROSS_COMPILE?=aarch64-linux-gnu-
NAME := Hw2

all: x86_build
clean: x86_clean RPI_clean

RPI_build:
	$(CROSS_COMPILE)gcc hw2.c -o hw2
	$(CROSS_COMPILE)gcc hw2_client.c -o hw2_client
RPI_clean:
	rm -f hw2
	rm -f hw2_client

x86_build:
	gcc hw2.c -o hw2
	gcc hw2_client.c -o hw2_client
x86_clean:
	rm -f hw2
	rm -f hw2_client

.PHONY: all clean x86_build x86_clean RPI_build RPI_clean zip

zip: all
	rm -rf $(NAME)
	rm -rf $(NAME).zip
	mkdir $(NAME)
	cp hw2.c $(NAME)
	cp hw2 $(NAME)
	cp Makefile $(NAME)
	cp hw2_checker $(NAME)
	cp hw2_client.c $(NAME)
	cp hw2_client $(NAME)
	zip -r $(NAME).zip $(NAME)
	rm -rf $(NAME)
	make clean
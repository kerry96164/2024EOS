all: x86_build
clean: x86_clean
x86_build:
	gcc server.c -o server
	gcc client.c -o client
x86_clean:
	rm -f server client

zip:
zip: all
	rm -rf Lab6
	rm -rf Lab6.zip
	mkdir Lab6
	cp demo.sh Lab6
	cp client.c Lab6
	cp server.c Lab6
	cp Makefile Lab6
	zip -r Lab6.zip Lab6
	rm -rf Lab6
	make clean
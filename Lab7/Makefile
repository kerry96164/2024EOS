all: x86_build
clean: x86_clean
x86_build:
	gcc game.c -o game
	gcc guess.c -o guess
x86_clean:
	rm -f game guess

zip:
zip: all
	rm -rf Lab7
	rm -rf Lab7.zip
	mkdir Lab7
	cp demo.sh Lab7
	cp game.c Lab7
	cp guess.c Lab7
	cp Makefile Lab7
	zip -r Lab7.zip Lab7
	rm -rf Lab7
	make clean
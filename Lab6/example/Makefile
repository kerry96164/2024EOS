all: build

build: clean
	gcc semaphore/makesem.c -o makesem
	gcc semaphore/rmsem.c -o rmsem
	gcc doodle.c -o doodle
	gcc pipe.c -o pipe
	gcc select.c -o select
clean:
	rm -f makesem rmsem doodle pipe select


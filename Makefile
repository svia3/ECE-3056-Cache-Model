CFLAGS ?= -O3
FILES = main.c cachesim.c cachesim.h Makefile
cachesim: main.o cachesim.o

cachesim.o: cachesim.c cachesim.h
main.o: main.c cachesim.h

clean:
	rm -f *.o *~ \#* cachesim

submit:
	tar -czvf last_first_a3_a.tar.gz $(FILES)


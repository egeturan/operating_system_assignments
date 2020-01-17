all: libmemalloc.a app

libmemalloc.a: memalloc.c
	gcc -Wall -c memalloc.c
	ar -cvq libmemalloc.a memalloc.o
	ranlib libmemalloc.a

app: app.c 
	gcc -Wall -o app app.c  -L. -lmemalloc -lpthread

clean:
	rm -fr *.o *.a *~ a.out app

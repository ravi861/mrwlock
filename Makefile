CC=gcc
CFLAGS= -g -fPIC -lrt -lpthread
PWD=$(shell pwd)


libmrw: clean
	gcc $(CFLAGS) -c -Werror mrwlock.c
	gcc $(CFLAGS) -shared -o libmrw.so mrwlock.o

mrw: clean libmrw thread_test.o
	gcc -L$(PWD) -Wl,-rpath=$(PWD) -o thread_test thread_test.c $(CFLAGS) -lmrw -DMRW

rw: clean thread_test.o
	gcc -o thread_test thread_test.c $(CFLAGS) -DRW

mutex: clean thread_test.o
	gcc -o thread_test thread_test.c $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf *.so
	rm -f thread_test



all: libsystopo.so main

main: src/main.cc
	g++ -o main src/main.cc -lsystopo -Iinclude -L.

libsystopo.so: systopo.o
	g++ -o libsystopo.so -shared src/systopo.o -Lsrc

systopo.o: include/systopo.h src/systopo.cc
	g++ -fPIC -Iinclude -c -o src/systopo.o src/systopo.cc

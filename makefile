# makefile

CC = g++

all: main

my_executable: main.o
	$(CC) -o my_executable main.o

main.o: main.cpp
	g++ -g -w -std=c++11 -c main.cpp

clean:
	rm -rf *.o *.csv main data*_*

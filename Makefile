CFLAGS = -ansi -pedantic -Wall -Werror

all:
	mkdir bin
	g++ $(CFLAGS) src/rshell.cpp -o bin/rshell

rshell:
	mkdir bin
	g++ $(CFLAGS) src/rshell.cpp -o bin/rshell
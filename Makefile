CFLAGS = -ansi -pedantic -Wall -Werror

all:
	rm -rfv bin
	mkdir bin
	g++ $(CFLAGS) src/rshell.cpp -o bin/rshell
	g++ $(CFLAGS) src/ls.cpp -o bin/ls

rshell:
	rm -rfv bin
	mkdir bin
	g++ $(CFLAGS) src/rshell.cpp -o bin/rshell

ls:
	rm -rfv bin
	mkdir bin
	g++ $(CFLAGS) src/ls.cpp -o bin/ls

clean:
	rm -rfv bin

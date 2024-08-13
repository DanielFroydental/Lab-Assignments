all: myshell

myshell: myshell.o
	gcc -Wall -g -m32 -o myshell myshell.o

myshell.o: myshell.c
	gcc -Wall -g -m32 -c myshell.c

.PHONY: clean
clean:
	rm -f myshell *.o

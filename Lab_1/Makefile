all: menuMap

menuMap: menu_map.c
	gcc -g -Wall -m32 -o menuMap menu_map.c

.PHONY: clean
clean:
	rm -f *.o menuMap

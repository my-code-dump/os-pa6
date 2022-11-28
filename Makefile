all: letsride

letsride: main.o random437.o
	gcc main.o random437.o -lm -o letsride

main.o: main.c
	gcc -c main.c

random437.o: random437.c 
	gcc -c random437.c

clean:
	-rm *.o letsride

DUMC := ../out/dumc
CC   := gcc

example: main.o simple
	$(CC) -o $@ main.o simple.o

main.o: main.c
	$(CC) -c -o $@ $^

simple:
	$(DUMC) simple.dum

clean:
	-rm -f main.o simple.o example

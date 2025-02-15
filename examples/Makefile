DUMC := ../out/dumc
CC   := gcc

example: main.o simple.o
	$(CC) -o $@ $^

main.o: main.c
	$(CC) -c -o $@ $^

simple.o: simple.dum
	$(DUMC) $^

clean:
	-rm -f main.o simple.o example

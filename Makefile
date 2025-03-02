CC = gcc -std=c17

SRCD = src
OBJD = obj
OUTD = out

BIN = $(OUTD)/dumc

SRC = $(wildcard $(SRCD)/*.c)
OBJ = $(patsubst $(SRCD)/%.c,$(OBJD)/%.o,$(SRC))

LIBS = libelf

CFLAGS = $(shell pkg-config --cflags $(LIBS)) -Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wconversion
LDFLAGS = $(shell pkg-config --libs $(LIBS))

debug: CFLAGS += -g3
debug: all

release: CFLAGS += -O3
release: all

all: $(OUTD) $(OBJD) $(BIN)

examples: all
	$(MAKE) -C examples/

$(BIN): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

$(OBJD)/%.o: $(SRCD)/%.c
	$(CC) -o $@ $(CFLAGS) -c $^

$(OBJD) $(OUTD):
	mkdir -p $@

clean:
	$(MAKE) -C examples/ clean
	-rm -rf $(OBJD) $(OUTD)

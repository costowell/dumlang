CC = c99

SRCD = src
OBJD = obj
OUTD = out

BIN = $(OUTD)/dumc

SRC = $(wildcard $(SRCD)/*.c)
OBJ = $(patsubst $(SRCD)/%.c,$(OBJD)/%.o,$(SRC))

CFLAGS = -Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wconversion
LDFLAGS =

debug: CFLAGS += -g3
debug: all

release: CFLAGS += -O3
release: all

all: $(OUTD) $(OBJD) $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

$(OBJD)/%.o: $(SRCD)/%.c
	$(CC) -o $@ $(CFLAGS) -c $^

$(OBJD) $(OUTD):
	mkdir -p $@

clean:
	-rm -r $(OBJD) $(OUTD)

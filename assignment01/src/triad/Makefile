SHELL:=/bin/bash
HOSTNAME:=$(shell hostname)
BIN:=triad
NMAX?=26

CC?=gcc
CFLAGS?=-fopenmp -Ofast -march=native

all: $(BIN)

$(BIN): triad.c Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

.PHONY: clean run
run: $(BIN)
	./$(BIN) $(NMAX)

clean:
	$(RM) $(BIN)

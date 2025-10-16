SHELL:=/bin/bash
HOSTNAME:=$(shell hostname)
BIN:=triad-$(HOSTNAME)
NMAX?=26

CC?=gcc
CFLAGS?=-fopenmp -O3 -march=native

all: $(BIN)

$(BIN): triad.c Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

.PHONY: clean run
run: $(BIN)
	./$(BIN) $(NMAX)

clean:
	$(RM) $(BIN)

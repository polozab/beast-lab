CC?=gcc
SHELL := /bin/bash

LIKWID_BASE:=$(shell dirname $(shell bash -lc 'command -v likwid-perfctr' 2>/dev/null))

LIKWID_LIB:=$(LIKWID_BASE)/../lib/
LIKWID_INCLUDE:=$(LIKWID_BASE)/../include/

CFLAGS ?= -fopenmp -O3 -ffast-math -march=native -DLIKWID_PERFMON -L$(LIKWID_LIB)
LDFLAGS += -I$(LIKWID_INCLUDE) -Wl,-rpath,$(LIKWID_INCLUDE)
LDLIBS += -llikwid

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.out)

all: $(OBJS)

%.out: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f $(OBJS)

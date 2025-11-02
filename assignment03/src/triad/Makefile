CC?=gcc

LIKWID_LIB:=$(shell dirname $(shell bash -lc 'which likwid-perfctr' 2>/dev/null))/../lib/
LIKWID_INCLUDE:=$(shell dirname $(shell bash -lc 'which likwid-perfctr' 2>/dev/null))/../include/

CFLAGS ?= -fopenmp -O3 -ffast-math -march=native -DLIKWID_PERFMON
LDFLAGS += -L$(LIKWID_LIB) -I$(LIKWID_INCLUDE) 
LDLIBS += -llikwid

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.out)

all: $(OBJS)

%.out: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f $(OBJS)

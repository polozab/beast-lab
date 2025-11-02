CC?=gcc

LIKWID_LIB:=$(dirname $(which likwid-perfctr))/../lib/"
LIKWID_INCLUDE:=$(dirname $(which likwid-perfctr))/../include/"

CFLAGS ?= -fopenmp -O3 -ffast-math -march=native -DLIKWID_PERFMON
LDFLAGS += -L$LIKWID_LIB -I$LIKWID_INCLUDE 
LDLIBS += -llikwid

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.out)

all: $(OBJS)

%.out: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f $(OBJS)

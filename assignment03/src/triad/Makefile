CC?=gcc

LIKWID_BASE := $(shell dirname $(dir $(shell command -v likwid - perfctr)))

CFLAGS?=-fopenmp -O3 -ffast-math -march=native
CPPFLAGS += -DLIKWID_PERFMON -I$(LIKWID_BASE)/include
LDFLAGS += -L$(LIKWID_BASE)/lib -Wl,-rpath,$(LIKWID_BASE)/lib -llikwid

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.out)

all: $(OBJS)

%.out: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(OBJS)

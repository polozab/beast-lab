CC?=gcc

LIKWID_BAS_BASE := $(shell bash -lc 'command -v likwid-perfctr' 2>/dev/null)
LIKWID_BASE := $(shell dirname $(dir $(LIKWID_BASE_BASE)))

CFLAGS ?= -fopenmp -O3 -ffast-math -march=native
CPPFLAGS += -DLIKWID_PERFMON -I$(LIKWID_BASE)/include
LDFLAGS += -L$(LIKWID_BASE)/lib -Wl,-rpath,$(LIKWID_BASE)/lib -llikwid

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.out)

all: $(OBJS)

%.out: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(OBJS)

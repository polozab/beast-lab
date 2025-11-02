CCX?=g++

LIKWID_BASE_BASE := $(shell bash -lc 'command -v likwid-perfctr' 2>/dev/null)
LIKWID_BASE := $(shell dirname $(dir $(LIKWID_BASE_BASE)))

CFLAGS?=-march=native -ffast-math -fstrict-aliasing -fopenmp
CPPFLAGS += -DLIKWID_PERFMON -I$(LIKWID_BASE)/include -std=c++17
LDFLAGS += -L$(LIKWID_BASE)/lib -Wl,-rpath,$(LIKWID_BASE)/lib -llikwid

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.out)

all: $(OBJS)

%.out: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(OBJS)

CCX?=g++

LIKWID_BASE := $( shell dirname $( dir $( shell command -v likwid - perfctr )))

CFLAGS?=-fopenmp -O3 -ffast-math -march=native
CPPFLAGS += -DLIKWID_PERFMON -I$(LIKWID_BASE)/include
LDFLAGS += -L$(LIKWID_BASE)/lib -Wl,-rpath,$(LIKWID_BASE)/lib -llikwid

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.out)

all: $(OBJS)

%.out: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(OBJS)

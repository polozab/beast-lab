CXX=clang++
CXXFLAGS= -std=c++17 -Wall -Wextra -fopenmp --offload-arch=gfx90a -O3 -ffast-math -funroll-loops -fstrict-aliasing
LDFLAGS= -L/opt/rocm-6.3.4/lib/libroctx64.so.4 -Wl,-rpath=/opt/rocm-6.3.4/lib -lroctx64

TILE_SIZE ?= 64
NUM_TEAMS ?= 64
NUM_THREADS ?= 64

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.out)

all: $(OBJS)

debug: CXXFLAGS += -DPRINT_DEBUG
debug: all

%.out: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DTILE_SIZE=$(TILE_SIZE) -DNUM_TEAMS=$(NUM_TEAMS) -DNUM_THREADS=$(NUM_THREADS) $< -o $@

clean:
	rm -f $(OBJS)

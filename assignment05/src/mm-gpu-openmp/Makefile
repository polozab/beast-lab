CXX=clang++
CXXFLAGS= -std=c++17 -Wall -Wextra -fopenmp --offload-arch=gfx90a -O3

NUM_TEAMS ?= 64
NUM_THREADS ?= 64

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.out)

all: $(OBJS)

debug: CXXFLAGS += -fopt-info -DPRINT_DEBUG
debug: all

%.out: %.cpp
	$(CXX) $(CXXFLAGS) -DNUM_TEAMS=$(NUM_TEAMS) -DNUM_THREADS=$(NUM_THREADS) $< -o $@

clean:
	rm -f $(OBJS)

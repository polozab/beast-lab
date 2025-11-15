CXX=clang++
CXXFLAGS= -std=c++17 -Wall -Wextra -fopenmp --offload-arch=gfx90a -O3

NUM_TEAMS ?= $(or $(firstword $(MAKECMDGOALS)), 64)
NUM_THREADS ?= $(or $(word 2, $(MAKECMDGOALS)), 64)

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.out)

all: $(OBJS)

debug:
	$(MAKE) $(NUM_TEAMS) $(NUM_THREADS) CXXFLAGS="$(CXXFLAGS) -g -DPRINT_DEBUG"

%.out: %.cpp
	$(CXX) $(CXXFLAGS) -DNUM_TEAMS=$(NUM_TEAMS) -DNUM_THREADS=$(NUM_THREADS) $< -o $@

clean:
	rm -f $(OBJS)

CXX=clang++
CXXFLAGS= -std=c++17 -Wall -Wextra -fopenmp --offload-arch=gfx90a -O3

NUM_TEAMS ?= $(firstword $(MAKECMDGOALS))
NUM_THREADS ?= $(word 2, $(MAKECMDGOALS))

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.out)

all: $(OBJS)

debug:
	$(MAKE) $(NUM_TEAMS) $(NUM_THREADS) CXXFLAGS="$(CXXFLAGS) -g -DPRINT_DEBUG"

%.out: %.cpp
	$(CXX) $(CXXFLAGS) -DNUM_TEAMS=$(NUM_TEAMS) -DNUM_THREADS=$(NUM_THREADS) $< -o $@

clean:
	rm -f $(OBJS)

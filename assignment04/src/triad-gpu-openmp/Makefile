CXX ?= nvc++
CXXFLAGS = -std=c++17 -fopenmp -mp=gpu

# or use llvm:
# CXX=clang++
# CXXFLAGS=-fopenmp -Ofast -std=c++17 -Wall -Wextra -g --offload-arch=sm_90
#
# need to set LD_LIBRARY_PATH for clang:
# export LD_LIBRARY_PATH=$(dirname $(dirname $(command -v clang)))/lib/aarch64-unknown-linux-gnu:$LD_LIBRARY_PATH

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.out)

all: $(OBJS)

%.out: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(OBJS)
  

CXX ?= nvc++
CXXFLAGS = -std=c++17 -fopenmp -mp=gpu -O3 -ffastmath

# or use llvm:
# CXX=clang++
# CXXFLAGS=-fopenmp -Ofast -std=c++17 -Wall -Wextra -g --offload-arch=sm_90
#
# need to set LD_LIBRARY_PATH for clang:
# export LD_LIBRARY_PATH=$(dirname $(dirname $(command -v clang)))/lib/aarch64-unknown-linux-gnu:$LD_LIBRARY_PATH

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.out)

all: $(OBJS)

%.out: %.c
  $(CXX) $(CXXFLAGS) $< -o $@

clean:
  rm -f $(OBJS)

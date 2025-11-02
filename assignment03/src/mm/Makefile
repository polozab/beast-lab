CXX?=c++
SHELL := /bin/bash

LIKWID_BASE:=$(shell dirname $(shell bash -lc 'command -v likwid-perfctr' 2>/dev/null))

LIKWID_LIB:=$(LIKWID_BASE)/../lib/
LIKWID_INCLUDE:=$(LIKWID_BASE)/../include/

CFLAGS ?= -fopenmp -O3 -ffast-math -march=native -DLIKWID_PERFMON
CPPFLAGS += -std=c++17
LDFLAGS += -I$(LIKWID_INCLUDE) -L$(LIKWID_LIB) -Wl,-rpath,$(LIKWID_LIB)
LDLIBS += -llikwid

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.out)

all: $(OBJS)

%.out: %.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f $(OBJS)

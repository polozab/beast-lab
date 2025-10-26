CXX?=c++
CXXFLAGS?=-Wall -Wextra -O3 -march=native -ffast-math -fstrict-aliasing -fopenmp -std=c++17

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.out)

all: $(OBJS)

assignment: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.out: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(OBJS)

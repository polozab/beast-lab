CXX?=c++
CXXFLAGS?=-Wall -Wextra -O3 -march=native -ffast-math -fstrict-aliasing -fopenmp -std=c++17

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

all: $(OBJS)

assignment: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)

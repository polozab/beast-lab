CXX?=c++
CXXFLAGS?=-Wall -Wextra -Ofast -march=native -std=c++17 -fopenmp -fstrict-aliasing

all: assignment

assignment:
	$(CXX) $(CXXFLAGS) -o $@ assignment.cpp

clean:
	rm -f assignment

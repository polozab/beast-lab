CXX=amdclang++
CXXFLAGS=-O3 -Wall -Rpass -target x86_64-linux-gnu -fopenmp --offload-arch=gfx90a

SRC=main.cpp
OUT=main

all: $(OUT)

$(OUT):
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) -lm

clean:
	rm $(OUT)

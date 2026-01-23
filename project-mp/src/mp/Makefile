CXX=amdclang++
CXXFLAGS=-O3 -Wall -Rpass -target x86_64-linux-gnu -fopenmp --offload-arch=gfx90a
CPPFLAGS=

ROCM_ROOT=/opt/rocm

CPPFLAGS+=-I$(ROCM_ROOT)/include/rocprofiler-sdk-roctx
LDFLAGS=-Wl,-rpath,$(ROCM_ROOT)/lib
LDLIBS=-L$(ROCM_ROOT)/lib -lroctx64 -lm

OBJS = main.o

all: mp

mp: $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) -o mp $(LDLIBS)

clean:
	rm mp mp.o

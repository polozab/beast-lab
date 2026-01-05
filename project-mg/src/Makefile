CXX?=nvc++
CXXFLAGS=-fast -Munroll -Mvect=simd -Minfo -Mnoautoinline -O2 -mp -gopt

NVTX_ROOT:=/opt/nvidia/hpc_sdk/Linux_aarch64/24.1/cuda

CXXINCLUDES=-I$(NVTX_ROOT)/include
LDFLAGS=-Wl,rpath,$(NVTX_ROOT)/lib64
LDLIBS=-L$(NVTX_ROOT)/lib64 -lnvtx3

OBJS = main.o

all: multigrid

multigrid: $(OBJS)
	$(CXX) $(CXXFLAGS) $(CXXINCLUDES) $(LDFLAGS) $(OBJS) -o multigrid $(LDLIBS)

clean:
	rm -f *~ *.o *.vtk multigrid


CXX?=nvc++
CXXFLAGS:=-fast -O4 -Minfo -Mnoautoinline -mp=multicore -gopt
CPPFLAGS:=

NVTX_ROOT:=/opt/nvidia/hpc_sdk/Linux_aarch64/24.1/cuda

CPPFLAGS+=-I$(NVTX_ROOT)/include
LDFLAGS:=-Wl,-rpath,$(NVTX_ROOT)/lib64
LDLIBS:=-L$(NVTX_ROOT)/lib64 -lnvToolsExt

OBJS = main.o

all: multigrid

multigrid: $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) -o multigrid $(LDLIBS)

clean:
	rm -f *~ *.o *.vtk multigrid

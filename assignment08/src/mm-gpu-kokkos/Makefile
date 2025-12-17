# KOKKOS_ROOT set via:
#   module load kokkos
# 
# You can look at the first lines of
# $KOKKOS_ROOT/lib64/cmake/Kokkos/KokkosConfigCommon.cmake to see the flags
# used in cmake configuration of the kokkos library build. The default Kokkos
# module on Aurora was built with the default oneAPI module and includes
# Serial, OpenMP (threads) and SYCL backends. So you should have that
# environment module loaded and include compiler flags for sycl and openmp:

# Aurora MPICH wrapper for C++ and C compilers:
CXX="icpx"
CC="icx"

SYCL_AOT_CPPFLAGS=-fsycl -fsycl-targets=spir64_gen -fno-sycl-id-queries-fit-in-int -fsycl-dead-args-optimization -fsycl-unnamed-lambda -std=c++20
SYCL_AOT_LDFLAGS=-Xsycl-target-backend "-device pvc"

CPPFLAGS=-g -O3 -fiopenmp -I $(KOKKOS_ROOT)/include $(SYCL_AOT_CPPFLAGS) -Wno-deprecated-declarations -Wno-tautological-constant-compare -Wno-unknown-attributes -ffp-model=precise

GCC_ROOT:=/home/sw/spack_new/spack_stack_dev/sap/spack/opt/spack/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder_/linux-sles15-icelake/gcc-13.2.0/gcc-14.2.0-57z6vikjlbeafvcvfuq5u65hgsjsi5bk

ONEAPI_ROOT:=/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib/

LDFLAGS=$(CPPFLAGS) -Wl,-rpath,$(GCC_ROOT)/lib64 $(SYCL_AOT_LDFLAGS)
LDLIBS=-L$(KOKKOS_ROOT)/lib64 -lkokkoscore -lkokkossimd -lkokkoscontainers -lpthread

SRCS=hello_kokkos.cpp kokkos_mm.cpp kokkos_device_props_sycl.cpp \
     kokkos_mm_ThreadsTeams.cpp \
     kokkos_mm_TT_RRR.cpp kokkos_mm_TT_RRL.cpp kokkos_mm_TT_RLR.cpp kokkos_mm_TT_RLL.cpp \
     kokkos_mm_TT_LRR.cpp kokkos_mm_TT_LRL.cpp kokkos_mm_TT_LLR.cpp kokkos_mm_TT_LLL.cpp tmp2.cpp kokkos_mm_GPU_Only.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: hello_kokkos kokkos_mm kokkos_device_props_sycl \
     kokkos_mm_ThreadsTeams \
     kokkos_mm_TT_RRR kokkos_mm_TT_RRL kokkos_mm_TT_RLR kokkos_mm_TT_RLL \
     kokkos_mm_TT_LRR kokkos_mm_TT_LRL kokkos_mm_TT_LLR kokkos_mm_TT_LLL tmp2 kokkos_mm_GPU_Only

hello_kokkos: hello_kokkos.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm: kokkos_mm.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_device_props_sycl: kokkos_device_props_sycl.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_ThreadsTeams: kokkos_mm_ThreadsTeams.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_TT_RRR: kokkos_mm_TT_RRR.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_TT_RRL: kokkos_mm_TT_RRL.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_TT_RLR: kokkos_mm_TT_RLR.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_TT_RLL: kokkos_mm_TT_RLL.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_TT_LRR: kokkos_mm_TT_LRR.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_TT_LRL: kokkos_mm_TT_LRL.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_TT_LLR: kokkos_mm_TT_LLR.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_TT_LLL: kokkos_mm_TT_LLL.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm_GPU_Only: kokkos_mm_GPU_Only.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
tmp2: tmp2.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)

run_all: hello_kokkos kokkos_mm kokkos_device_props_sycl
	ONEAPI_DEVICE_SELECTOR=ext_oneapi_level_zero:gpu ./hello_kokkos
	ONEAPI_DEVICE_SELECTOR=ext_oneapi_level_zero:gpu ./kokkos_mm 512
	ONEAPI_DEVICE_SELECTOR=ext_oneapi_level_zero:gpu ./kokkos_device_props_sycl

clean:
	rm -f $(OBJS)

distclean: clean
	rm -f hello_kokkos kokkos_mm kokkos_device_props_sycl 

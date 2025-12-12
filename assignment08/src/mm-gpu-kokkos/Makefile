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

LDFLAGS=$(CPPFLAGS) $(SYCL_AOT_LDFLAGS) -Wl,-rpath,$(GCC_ROOT)/lib64
LDLIBS=-L$(KOKKOS_ROOT)/lib64 -lkokkoscore -lkokkossimd -lkokkoscontainers -lpthread

SRCS=hello_kokkos.cpp kokkos_mm.cpp kokkos_device_props_sycl.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: hello_kokkos kokkos_mm kokkos_device_props_sycl

hello_kokkos: hello_kokkos.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_mm: kokkos_mm.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)
kokkos_device_props_sycl: kokkos_device_props_sycl.o
	$(CXX) $(LDFLAGS) -o $@ $< $(LDLIBS)

run_all: hello_kokkos kokkos_mm kokkos_device_props_sycl
	ONEAPI_DEVICE_SELECTOR=ext_oneapi_level_zero:gpu ./hello_kokkos
	ONEAPI_DEVICE_SELECTOR=ext_oneapi_level_zero:gpu ./kokkos_mm 512
	ONEAPI_DEVICE_SELECTOR=ext_oneapi_level_zero:gpu ./kokkos_device_props_sycl

clean:
	rm -f $(OBJS)

distclean: clean
	rm -f hello_kokkos kokkos_mm kokkos_device_props_sycl 

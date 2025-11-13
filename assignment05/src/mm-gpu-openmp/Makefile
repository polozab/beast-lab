NODE=$(shell uname -a |  cut -f2 -d" ")

ifneq (,$(filter $(NODE),gh1 gh2 gh3 gh4)) # grace-hopper system
        CXX=nvc++
        CXXFLAGS=-std=c++17 -fopenmp -mp=gpu -O3 -g
        # or use llvm:
        # CXX=clang++
        # CXXFLAGS=-fopenmp -Ofast -std=c++17 -Wall -Wextra -g --offload-arch=sm_90
        #
        # need to set LD_LIBRARY_PATH for clang:
        # export LD_LIBRARY_PATH=$(dirname $(dirname $(command -v clang)))/lib/aarch64-unknown-linux-gnu:$LD_LIBRARY_PATH
        BINARY_POSTFIX=gh
else ifneq (,$(filter $(NODE),ice1 ice2)) # intel ice
        CXX=nvc++
        CXXFLAGS=-std=c++17 -fopenmp -mp=gpu -O3 -g
        BINARY_POSTFIX=ice
else ifneq (,$(filter $(NODE),sap3 sap4)) # intel sap
        CXX=icpx
        CXXFLAGS=-std=c++17 -fsycl -fiopenmp -fopenmp-targets=spir64 -O3
        BINARY_POSTFIX=sap
else ifneq (,$(filter $(NODE),rome1 rome2)) # amd rome
        CXX=/opt/rocm/llvm/bin/clang++
        CXXFLAGS= -std=c++17 -Wall -Wextra -fopenmp -target x86_64-pc-linux-gnu -fopenmp-targets=amdgcn-amd-amdhsa -Xopenmp-target=amdgcn-amd-amdhsa -march=gfx908 -O3 -g
        BINARY_POSTFIX=rome
else # milan system
        CXX=clang++
        CXXFLAGS= -std=c++17 -Wall -Wextra -fopenmp --offload-arch=gfx90a -O3 -g
        BINARY_POSTFIX=milan
endif

all: mm-gpu-offload

mm-gpu-offload: mm-gpu-offload.cpp
	${CXX} ${CXXFLAGS} mm-gpu-offload.cpp -o mm-gpu-offload-${BINARY_POSTFIX}
		
clean:
	rm -f mm-gpu-offload-${BINARY_POSTFIX}
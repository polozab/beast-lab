#! /usr/bin/sh

CMD_GCC="-std=c++17 -o assignment assignment.cpp"
CMF_CC="-lstdc++ -o assignment assignment.cpp"
module load cce/11.0.0
module load gcc

## a) Compiler flags

### CXX

$CXX -O3 $CMD_GCC
#./assignment > ./data/cxx/o3.out
#./assignment >> ./data/cxx/o3.out

$CXX -O3 -march=native $CMD_GCC
#./assignment > ./data/cxx/o3_native.out
#./assignment >> ./data/cxx/o3.out

$CXX -O3 -march=native -ffast-math $CMD_GCC
#./assignment > ./data/cxx/o3_native_fast-math.out
#./assignment >> ./data/cxx/o3_native_fast-math.out

$CXX -O3 -march=native -ffast-math -fstrict-aliasing $CMD_GCC
#./assignment > ./data/cxx/o3_native_fast-math_strict-aliasing.out
#./assignment >> ./data/cxx/o3_native_fast-math_strict-aliasing.out

$CXX -O3 -march=native -ffast-math -fstrict-aliasing -fopenmp $CMD_GCC
#./assignment > ./data/cxx/o3_native_fast-math_strict-aliasing_fopenmp.out
#./assignment >> ./data/cxx/o3_native_fast-math_strict-aliasing_fopenmp.out

$CXX -Ofast -march=native $CMD_GCC
#./assignment > ./data/cxx/ofast_native.out
#./assignment >> ./data/cxx/ofast_native.out

$CXX -O3 $CMD_GCC
#./assignment > ./data/cxx/o3.out
#./assignment >> ./data/cxx/o3.out

### CC

cc -O3 $CMD_CC
#./assignment > ./data/cc/o3.out
#./assignment >> ./data/cc/o3.out

cc -O3 -march=armv8 $CMD_CC
#./assignment > ./data/cc/o3_native_.out
#./assignment >> ./data/cc/o3_native_.out

cc -O3 -march=armv8 -ffast-math $CMD_CC
#./assignment > ./data/cc/o3_native_fast-math.out
#./assignment >> ./data/cc/o3_native_fast-math.out

cc -O3 -march=armv8 -ffast-math -fstrict-aliasing $CMD_CC
#./assignment > ./data/cc/o3_native_fast-math_strict-aliasing.out
#./assignment >> ./data/cc/o3_native_fast-math_strict-aliasing.out

cc -O3 -march=armv8 -ffast-math -fstrict-aliasing -fopenmp $CMD_CC
#./assignment > ./data/cc/o3_native_fast-math_strict-aliasing_fopenmp.out
#./assignment >> ./data/cc/o3_native_fast-math_strict-aliasing_fopenmp.out

cc -Ofast -march=native $CMD_GCC
#./assignment > ./data/cc/ofast_native.out
#./assignment >> ./data/cc/ofast_native_.out

## b) 
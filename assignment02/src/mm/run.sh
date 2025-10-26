#! /usr/bin/sh

CMD_GCC="-std=c++17 -o assignment assignment.cpp"
CMD_CC="-lstdc++ -o assignment assignment.cpp"

## a) Compiler flags

### CXX
module load gcc

$CXX -O3 $CMD_GCC
echo Started Test 1
./assignment > ./data/cxx/o3.out
./assignment >> ./data/cxx/o3.out

$CXX -O3 -march=native $CMD_GCC
echo Started Test 2
./assignment > ./data/cxx/o3_native.out
./assignment >> ./data/cxx/o3.out

$CXX -O3 -march=native -ffast-math $CMD_GCC
echo Started Test 3
./assignment > ./data/cxx/o3_native_fast-math.out
./assignment >> ./data/cxx/o3_native_fast-math.out

$CXX -O3 -march=native -ffast-math -fstrict-aliasing $CMD_GCC
echo Started Test 4
./assignment > ./data/cxx/o3_native_fast-math_strict-aliasing.out
./assignment >> ./data/cxx/o3_native_fast-math_strict-aliasing.out

$CXX -O3 -march=native -ffast-math -fstrict-aliasing -fopenmp $CMD_GCC
echo Started Test 5
./assignment > ./data/cxx/o3_native_fast-math_strict-aliasing_fopenmp.out
./assignment >> ./data/cxx/o3_native_fast-math_strict-aliasing_fopenmp.out

$CXX -Ofast -march=native $CMD_GCC
echo Started Test 6
./assignment > ./data/cxx/ofast_native.out
./assignment >> ./data/cxx/ofast_native.out

$CXX -O3 $CMD_GCC
echo Started Test 7
./assignment > ./data/cxx/o3.out
./assignment >> ./data/cxx/o3.out

### CC
module load cce/11.0.0

cc -O3 $CMD_CC
echo Started Test 8
./assignment > ./data/cc/o3.out
./assignment >> ./data/cc/o3.out

cc -O3 -march=armv8 $CMD_CC
echo Started Test 9
./assignment > ./data/cc/o3_native_.out
./assignment >> ./data/cc/o3_native_.out

cc -O3 -march=armv8 -ffast-math $CMD_CC
echo Started Test 10
./assignment > ./data/cc/o3_native_fast-math.out
./assignment >> ./data/cc/o3_native_fast-math.out

cc -O3 -march=armv8 -ffast-math -fstrict-aliasing $CMD_CC
echo Started Test 11
./assignment > ./data/cc/o3_native_fast-math_strict-aliasing.out
./assignment >> ./data/cc/o3_native_fast-math_strict-aliasing.out

cc -O3 -march=armv8 -ffast-math -fstrict-aliasing -fopenmp $CMD_CC
echo Started Test 12
./assignment > ./data/cc/o3_native_fast-math_strict-aliasing_fopenmp.out
./assignment >> ./data/cc/o3_native_fast-math_strict-aliasing_fopenmp.out

cc -Ofast -march=armv8 $CMD_CC
echo Started Test 13
./assignment > ./data/cc/ofast_native.out
./assignment >> ./data/cc/ofast_native_.out

## b) 
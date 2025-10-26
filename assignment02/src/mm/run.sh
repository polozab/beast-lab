#! /usr/bin/sh

CMD="-std=c++17 -o assignment assignment.cpp"
module load gcc

## a) Compiler flags

$CXX -O3 $CMD
./assignment > o3.out
./assignment >> o3.out

$CXX -O3 -march=native $CMD
./assignment > o3.out
./assignment >> o3.out

$CXX -O3 -march=native -ffast-math $CMD
./assignment > o3_fast-math.out
./assignment >> o3_fast-math.out

$CXX -O3 -march=native -ffast-math -fstrict-aliasing $CMD
./assignment > o3_fast-math_strict-aliasing.out
./assignment >> o3_fast-math_strict-aliasing.out

$CXX -O3 --march=native ffast-math -fstrict-aliasing -fopenmp $CMD
./assignment > o3_fast-math_strict-aliasing_fopenmp.out
./assignment >> o3_fast-math_strict-aliasing_fopenmp.out

$CXX -Ofast -march=native $CMD
./assignment > ofast.out
./assignment >> ofast.out

## b) 
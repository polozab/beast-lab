#!/usr/bin/bash

#SBATCH -J triad
#Output and error (also --output, --error):
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#Initial working directory (also --chdir):
#SBATCH -D ./
#Notification and type
#SBATCH --time=00:30:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=milan #milan sap gh cs
#SBATCH --nodelist=milan1 #sap4 gh3 cs7
#SBATCH --nodes=1

module load gcc/14.2.0-gcc-13.2.0-4wqwz2p
module load likwid

make clean
make

BASE="./data/milan2"

rm -r "$BASE"
mkdir -p "$BASE"

stdbuf --output=L $CC -v | tee "$BASE/_compiler.txt"

likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 9 > "$BASE/FLOPS_T1_L1.txt"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 13 > "$BASE/FLOPS_T1_L2.txt"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 20 > "$BASE/FLOPS_T1_L3.txt"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 29 > "$BASE/FLOPS_T1_MEM.txt"

likwid-perfctr -C 0 -g L2 -m ./triad.out 9 > "$BASE/L2-L1_T1_L1.txt"
likwid-perfctr -C 0 -g L2 -m ./triad.out 13 > "$BASE/L2-L1_T1_L2.txt"
likwid-perfctr -C 0 -g L2 -m ./triad.out 20 > "$BASE/L2-L1_T1_L3.txt"
likwid-perfctr -C 0 -g L2 -m ./triad.out 29 > "$BASE/L2-L1_T1_MEM.txt"

likwid-perfctr -C 0 -g L3 -m ./triad.out 9 > "$BASE/L3-L2_T1_L1.txt"
likwid-perfctr -C 0 -g L3 -m ./triad.out 13 > "$BASE/L3-L2_T1_L2.txt"
likwid-perfctr -C 0 -g L3 -m ./triad.out 20 > "$BASE/L3-L2_T1_L3.txt"
likwid-perfctr -C 0 -g L3 -m ./triad.out 29 > "$BASE/L3-L2_T1_MEM.txt"

likwid-perfctr -C 0 -g MEM -m ./triad.out 9 > "$BASE/MEM-L3_T1_L1.txt"
likwid-perfctr -C 0 -g MEM -m ./triad.out 13 > "$BASE/MEM-L3_T1_L2.txt"
likwid-perfctr -C 0 -g MEM -m ./triad.out 20 > "$BASE/MEM-L3_T1_L3.txt"
likwid-perfctr -C 0 -g MEM -m ./triad.out 29 > "$BASE/MEM-L3_T1_MEM.txt"

likwid-perfctr -C 0-127 -g FLOPS_DP -m ./triad.out 9 > "$BASE/FLOPS_T128_L1.txt"
likwid-perfctr -C 0-127 -g FLOPS_DP -m ./triad.out 13 > "$BASE/FLOPS_T128_L2.txt"
likwid-perfctr -C 0-127 -g FLOPS_DP -m ./triad.out 18 > "$BASE/FLOPS_T128_L3.txt"
likwid-perfctr -C 0-127 -g FLOPS_DP -m ./triad.out 29 > "$BASE/FLOPS_T128_MEM.txt"

likwid-perfctr -C 0-127 -g L2 -m ./triad.out 9 > "$BASE/L2-L1_T128_L1.txt"
likwid-perfctr -C 0-127 -g L2 -m ./triad.out 13 > "$BASE/L2-L1_T128_L2.txt"
likwid-perfctr -C 0-127 -g L2 -m ./triad.out 18 > "$BASE/L2-L1_T128_L3.txt"
likwid-perfctr -C 0-127 -g L2 -m ./triad.out 29 > "$BASE/L2-L1_T128_MEM.txt"

likwid-perfctr -C 0-127 -g L3 -m ./triad.out 9 > "$BASE/L3-L2_T128_L1.txt"
likwid-perfctr -C 0-127 -g L3 -m ./triad.out 13 > "$BASE/L3-L2_T128_L2.txt"
likwid-perfctr -C 0-127 -g L3 -m ./triad.out 18 > "$BASE/L3-L2_T128_L3.txt"
likwid-perfctr -C 0-127 -g L3 -m ./triad.out 29 > "$BASE/L3-L2_T128_MEM.txt"

likwid-perfctr -C 0-127 -g MEM -m ./triad.out 9 > "$BASE/MEM-L3_T128_L1.txt"
likwid-perfctr -C 0-127 -g MEM -m ./triad.out 13 > "$BASE/MEM-L3_T128_L2.txt"
likwid-perfctr -C 0-127 -g MEM -m ./triad.out 18 > "$BASE/MEM-L3_T128_L3.txt"
likwid-perfctr -C 0-127 -g MEM -m ./triad.out 29 > "$BASE/MEM-L3_T128_MEM.txt"

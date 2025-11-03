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
#SBATCH --partition=cs #milan sap gh cs
#SBATCH --nodelist=cs7 #sap4 gh3 cs7
#SBATCH --nodes=1

module load gcc-13.1.0-gcc-8.4.1-djjcsay
module load likwid

make clean
make

BASE="./data/cs2"

rm -r "$BASE"
mkdir -p "$BASE"

stdbuf --output=L $CC -v | tee "$BASE/_compiler.txt"

likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 10 > "$BASE/FLOPS_T1_L1.txt"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 17 > "$BASE/FLOPS_T1_L2.txt"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 29 > "$BASE/FLOPS_T1_MEM.txt"

likwid-perfctr -C 0 -g L2 -m ./triad.out 10 > "$BASE/L2-L1_T1_L1.txt"
likwid-perfctr -C 0 -g L2 -m ./triad.out 17 > "$BASE/L2-L1_T1_L2.txt"
likwid-perfctr -C 0 -g L2 -m ./triad.out 29 > "$BASE/L2-L1_T1_MEM.txt"

likwid-perfctr -C 0 -g MEM -m ./triad.out 10 > "$BASE/MEM-L3_T1_L1.txt"
likwid-perfctr -C 0 -g MEM -m ./triad.out 17 > "$BASE/MEM-L3_T1_L2.txt"
likwid-perfctr -C 0 -g MEM -m ./triad.out 29 > "$BASE/MEM-L3_T1_MEM.txt"

likwid-perfctr -C 0-47 -g FLOPS_DP -m ./triad.out 10 > "$BASE/FLOPS_T48_L1.txt"
likwid-perfctr -C 0-47 -g FLOPS_DP -m ./triad.out 15 > "$BASE/FLOPS_T48_L2.txt"
likwid-perfctr -C 0-47 -g FLOPS_DP -m ./triad.out 29 > "$BASE/FLOPS_T48_MEM.txt"

likwid-perfctr -C 0-47 -g L2 -m ./triad.out 10 > "$BASE/L2-L1_T48_L1.txt"
likwid-perfctr -C 0-47 -g L2 -m ./triad.out 15 > "$BASE/L2-L1_T48_L2.txt"
likwid-perfctr -C 0-47 -g L2 -m ./triad.out 29 > "$BASE/L2-L1_T48_MEM.txt"

likwid-perfctr -C 0-47 -g MEM -m ./triad.out 9 > "$BASE/MEM-L3_T48_L1.txt"
likwid-perfctr -C 0-47 -g MEM -m ./triad.out 15 > "$BASE/MEM-L3_T48_L2.txt"
likwid-perfctr -C 0-47 -g MEM -m ./triad.out 29 > "$BASE/MEM-L3_T48_MEM.txt"

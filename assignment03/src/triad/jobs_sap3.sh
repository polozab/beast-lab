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
#SBATCH --partition=sap #milan sap gh cs
#SBATCH --nodelist=sap3 #sap4 gh3 cs7
#SBATCH --nodes=1

module load gcc/14.2.0-gcc-13.2.0-57z6vik
module load likwid

make clean
make

BASE="./data/sap3"

rm -r "$BASE"
mkdir -p "$BASE"

stdbuf --output=L $CC -v | tee "$BASE/_compiler.txt"

likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 10 > "$BASE/FLOPS_T1_L1.txt"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 16 > "$BASE/FLOPS_T1_L2.txt"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 21 > "$BASE/FLOPS_T1_L3.txt"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 29 > "$BASE/FLOPS_T1_MEM.txt"

likwid-perfctr -C 0 -g L2 -m ./triad.out 10 > "$BASE/L2-L1_T1_L1.txt"
likwid-perfctr -C 0 -g L2 -m ./triad.out 16 > "$BASE/L2-L1_T1_L2.txt"
likwid-perfctr -C 0 -g L2 -m ./triad.out 21 > "$BASE/L2-L1_T1_L3.txt"
likwid-perfctr -C 0 -g L2 -m ./triad.out 29 > "$BASE/L2-L1_T1_MEM.txt"

likwid-perfctr -C 0 -g L3 -m ./triad.out 10 > "$BASE/L3-L2_T1_L1.txt"
likwid-perfctr -C 0 -g L3 -m ./triad.out 16 > "$BASE/L3-L2_T1_L2.txt"
likwid-perfctr -C 0 -g L3 -m ./triad.out 21 > "$BASE/L3-L2_T1_L3.txt"
likwid-perfctr -C 0 -g L3 -m ./triad.out 29 > "$BASE/L3-L2_T1_MEM.txt"

likwid-perfctr -C 0 -g MEM -m ./triad.out 10 > "$BASE/MEM-L3_T1_L1.txt"
likwid-perfctr -C 0 -g MEM -m ./triad.out 16 > "$BASE/MEM-L3_T1_L2.txt"
likwid-perfctr -C 0 -g MEM -m ./triad.out 21 > "$BASE/MEM-L3_T1_L3.txt"
likwid-perfctr -C 0 -g MEM -m ./triad.out 29 > "$BASE/MEM-L3_T1_MEM.txt"

likwid-perfctr -C 0-95 -g FLOPS_DP -m ./triad.out 10 > "$BASE/FLOPS_T96_L1.txt"
likwid-perfctr -C 0-95 -g FLOPS_DP -m ./triad.out 16 > "$BASE/FLOPS_T96_L2.txt"
likwid-perfctr -C 0-95 -g FLOPS_DP -m ./triad.out 16 > "$BASE/FLOPS_T96_L3.txt"
likwid-perfctr -C 0-95 -g FLOPS_DP -m ./triad.out 29 > "$BASE/FLOPS_T96_MEM.txt"

likwid-perfctr -C 0-95 -g L2 -m ./triad.out 10 > "$BASE/L2-L1_T96_L1.txt"
likwid-perfctr -C 0-95 -g L2 -m ./triad.out 16 > "$BASE/L2-L1_T96_L2.txt"
likwid-perfctr -C 0-95 -g L2 -m ./triad.out 16 > "$BASE/L2-L1_T96_L3.txt"
likwid-perfctr -C 0-95 -g L2 -m ./triad.out 29 > "$BASE/L2-L1_T96_MEM.txt"

likwid-perfctr -C 0-95 -g L3 -m ./triad.out 10 > "$BASE/L3-L2_T96_L1.txt"
likwid-perfctr -C 0-95 -g L3 -m ./triad.out 16 > "$BASE/L3-L2_T96_L2.txt"
likwid-perfctr -C 0-95 -g L3 -m ./triad.out 16 > "$BASE/L3-L2_T96_L3.txt"
likwid-perfctr -C 0-95 -g L3 -m ./triad.out 29 > "$BASE/L3-L2_T96_MEM.txt"

likwid-perfctr -C 0-95 -g MEM -m ./triad.out 10 > "$BASE/MEM-L3_T96_L1.txt"
likwid-perfctr -C 0-95 -g MEM -m ./triad.out 16 > "$BASE/MEM-L3_T96_L2.txt"
likwid-perfctr -C 0-95 -g MEM -m ./triad.out 16 > "$BASE/MEM-L3_T96_L3.txt"
likwid-perfctr -C 0-95 -g MEM -m ./triad.out 29 > "$BASE/MEM-L3_T96_MEM.txt"

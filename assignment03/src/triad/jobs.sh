#!/usr/bin/bash

#SBATCH -J triad
#Output and error (also --output, --error):
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#Initial working directory (also --chdir):
#SBATCH -D ./
#Notification and type
#SBATCH --time=00:15:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=sap #milan sap gh cs
#SBATCH --nodelist=sap3 #sap4 gh3 cs7
#SBATCH --nodes=1

module load gcc
module load likwid

make clean
make

rm -r "./data"
mkdir -p "./data"

likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 10 > "./data/FLOPS_T1_L1"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 15 > "./data/FLOPS_T1_L2"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 21 > "./data/FLOPS_T1_L3"
likwid-perfctr -C 0 -g FLOPS_DP -m ./triad.out 29 > "./data/FLOPS_T1_MEM"
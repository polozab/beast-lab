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

mkdir -p "./data"

likwid-perfctr -C 0 -g MEM -m ./triad.out 10 > "./data/MEM_T1_L1"
likwid-perfctr -C 0 -g MEM -m ./triad.out 15 > "./data/MEM_T1_L2"
likwid-perfctr -C 0 -g MEM -m ./triad.out 21 > "./data/MEM_T1_L3"
likwid-perfctr -C 0 -g MEM -m ./triad.out 29 > "./data/MEM_T1_MEM"

likwid-perfctr -C 0 -g L2 -m ./triad.out 10 > "./data/L2_T1_L1"
likwid-perfctr -C 0 -g L2 -m ./triad.out 15 > "./data/L2_T1_L2"
likwid-perfctr -C 0 -g L2 -m ./triad.out 21 > "./data/L2_T1_L3"
likwid-perfctr -C 0 -g L2 -m ./triad.out 29 > "./data/L2_T1_MEM"

likwid-perfctr -C 0 -g L3 -m ./triad.out 10 > "./data/L3_T1_L1"
likwid-perfctr -C 0 -g L3 -m ./triad.out 15 > "./data/L3_T1_L2"
likwid-perfctr -C 0 -g L3 -m ./triad.out 21 > "./data/L3_T1_L3"
likwid-perfctr -C 0 -g L3 -m ./triad.out 29 > "./data/L3_T1_MEM"

likwid-perfctr -C 0-95 -g MEM -m ./triad.out 10 > "./data/MEM_T96_L1"
likwid-perfctr -C 0-95 -g MEM -m ./triad.out 15 > "./data/MEM_T96_L2"
likwid-perfctr -C 0-95 -g MEM -m ./triad.out 21 > "./data/MEM_T96_L3"
likwid-perfctr -C 0-95 -g MEM -m ./triad.out 29 > "./data/MEM_T96_MEM"

likwid-perfctr -C 0-95 -g L2 -m ./triad.out 10 > "./data/L2_T96_L1"
likwid-perfctr -C 0-95 -g L2 -m ./triad.out 15 > "./data/L2_T96_L2"
likwid-perfctr -C 0-95 -g L2 -m ./triad.out 21 > "./data/L2_T96_L3"
likwid-perfctr -C 0-95 -g L2 -m ./triad.out 29 > "./data/L2_T96_MEM"

likwid-perfctr -C 0-95 -g L3 -m ./triad.out 10 > "./data/L3_T96_L1"
likwid-perfctr -C 0-95 -g L3 -m ./triad.out 15 > "./data/L3_T96_L2"
likwid-perfctr -C 0-95 -g L3 -m ./triad.out 16 > "./data/L3_T96_L3"
likwid-perfctr -C 0-95 -g L3 -m ./triad.out 29 > "./data/L3_T96_MEM"
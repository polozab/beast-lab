#!/usr/bin/bash

#SBATCH -J job_t1
#SBATCH -D ./
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#SBATCH --time=01:00:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=sap #milan sap gh cs
#SBATCH --nodelist=sap4 #sap4 gh3 cs7
#SBATCH --nodes=1


stdbuf -oL ./run.sh | tee ./data/kokkos_kernel_mm.csv

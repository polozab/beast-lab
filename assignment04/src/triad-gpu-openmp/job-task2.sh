#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J a4_t2
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
#SBATCH --partition=gh #milan sap gh cs
#SBATCH --nodelist=gh3 #sap4 gh3 cs7
#SBATCH --nodes=1


module load nvhpc/25.3
make clean
make

mkdir -p "./data/memory-init"

./triad-gpu-offload.out 30 > "./data/memory-init/cpu.csv"
./triad-gpu-offload_gpu-init.out 30 > "./data/memory-init/gpu.csv"

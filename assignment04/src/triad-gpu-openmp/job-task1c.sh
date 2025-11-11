#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J a4_t1b
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

mkdir data

./triad-gpu-offload_static-1.out 30 > "./data/scheduling_static-1.csv"
./triad-gpu-offload_static.out 30 > "./data/scheduling_static.csv"
./triad-gpu-offload_dynamic.out 30 > "./data/scheduling_dynamic.csv"
./triad-gpu-offload_guided.out 30 > "./data/scheduling_guided.csv"
./triad-gpu-offload_auto.out 30 > "./data/scheduling_auto.csv"

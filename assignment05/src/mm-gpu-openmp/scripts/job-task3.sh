#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J a5t3v1
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
#SBATCH --nodelist=milan2 #sap4 gh3 cs7
#SBATCH --nodes=1


for teams in 8 16 32 64 128 256 512 1024; do 
    for threads in 8 16 32 64 128 256 512 1024; do
        make clean
        make NUM_THREADS=${threads} NUM_TEAMS=${teams} -B
        ./mm-gpu-offload_explicit_col_mod_task3.cpp  > "/data/teams_${teams}_${threads}.csv"
    done
done






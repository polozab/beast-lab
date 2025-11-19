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
#SBATCH --nodelist=milan1 #sap4 gh3 cs7
#SBATCH --nodes=1


for teams in 1024 512 256 128 64 32 16 8; do 
    for threads in 1024 512 256 128 64 32 16 8; do
        make clean
        make NUM_THREADS=${threads} NUM_TEAMS=${teams} -B
        ./mm-gpu-offload_explicit_col_mod_task3.out  > "data/teams_${teams}_${threads}.csv"
    done
done






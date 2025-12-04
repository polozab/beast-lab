#!/usr/bin/bash

#SBATCH -J job_t1
#Initial working directory (also --chdir):
#SBATCH -D ~/beastlab/assignment07/src/latency
#Notification and type
#SBATCH --time=00:30:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=gh #milan sap gh cs
#SBATCH --nodelist=gh3 #sap4 gh3 cs7
#SBATCH --nodes=1

module load nvhpc
nvc -Wall -Wextra -O3 -march=native -g list.c -o list_gh3.out
./list_gh3.out 30 > ./data/gh3.csv
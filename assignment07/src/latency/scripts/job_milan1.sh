#!/usr/bin/bash

#SBATCH -J job_t1
#Initial working directory (also --chdir):
#SBATCH -D ~/beastlab/assignment07/src/latency
#Notification and type
#SBATCH --time=00:30:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=milan #milan sap gh cs
#SBATCH --nodelist=milan1 #sap4 gh3 cs7
#SBATCH --nodes=1

clang -Wall -Wextra -Wconversion -O3 -march=native -g list.c -o list_milan1.out
./list_milan1.out 30 > ./data/milan1.csv
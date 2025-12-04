#!/usr/bin/bash

#SBATCH -J job_t1
#SBATCH -D ~/beastlab/assignment07/src/latency
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#SBATCH --time=00:30:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=cs #milan sap gh cs
#SBATCH --nodelist=cs7 #sap4 gh3 cs7
#SBATCH --nodes=1

module load cce/11.0.0
cc -Wall -Wextra -Wconversion -O3 -g list.c -o list_cs7.out
./list_cs7.out 30 > ./data/cs7.csv
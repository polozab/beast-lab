#!/usr/bin/bash

#SBATCH -J job_t1
#SBATCH -D ./
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#SBATCH --time=00:30:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=gh #milan sap gh cs
#SBATCH --nodelist=gh3 #sap4 gh3 cs7
#SBATCH --nodes=1

module load nvhpc
nvc -Wall -Wextra -O3 -march=native -g list.c -o list_gh3.out
taskset -c 0 stdbuf -oL ./list_gh3.out 30 | tee ./data/gh3.csv

nvc -Wall -Wextra -O3 -march=native -g list_single.c -o list_single_gh3.out
taskset -c 0 stdbuf -oL ./list_single_gh3.out 26 | tee ./data/gh3_numa.csv

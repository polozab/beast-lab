#!/usr/bin/bash

#SBATCH -J job_t1
#SBATCH -D ./
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#SBATCH --time=00:30:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=milan #milan sap gh cs
#SBATCH --nodelist=milan1 #sap4 gh3 cs7
#SBATCH --nodes=1

clang -Wall -Wextra -Wconversion -O3 -march=native -g list.c -o list_milan1.out
taskset -c 0 stdbuf -oL ./list_milan1.out 30 | tee ./data/milan1.csv

#!/usr/bin/bash

#SBATCH -J job_t1
#SBATCH -D ./
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#SBATCH --time=00:30:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=sap #milan sap gh cs
#SBATCH --nodelist=sap2 #sap4 gh3 cs7
#SBATCH --nodes=1

module load compiler
icx -Wall -Wextra -Wconversion -O3 -march=native -g list.c -o list_sap2.out
stdbuf -oL taskset -c 0 ./list_sap2.out 30 | tee ./data/sap2.csv

icx -Wall -Wextra -Wconversion -O3 -march=native -g list_single.c -o list_single_sap2.out
stdbuf -oL taskset -c 0 ./list_single_sap2.out 26 | tee ./data/sap2_numa.csv

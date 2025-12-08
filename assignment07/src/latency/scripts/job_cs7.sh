#!/usr/bin/bash

#SBATCH -J job_t1
#SBATCH -D ./
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
stdbuf -oL perf stat -o /dev/stdout taskset -c 0 ./list_cs7.out 30 | tee ./data/cs7.csv

cc -Wall -Wextra -Wconversion -O3 -g list_single.c -o list_single_cs7.out
stdbuf -oL perf stat -o /dev/stdout numactl -a -C 0 -m 2 ./list_single_cs7.out 26 | tee ./data/cs7_numa.csv

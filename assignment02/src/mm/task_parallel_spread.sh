#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J job_tiled_L1_t1
#Output and error (also --output, --error):
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#Initial working directory (also --chdir):
#SBATCH -D /Group-101/assignment02/src/mm/
#Notification and type
#SBATCH --time=12:00:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=cs #milan sap gh cs
#SBATCH --nodelist=cs2 #sap4 gh3 cs7
#SBATCH --nodes=1

# embed your test codes in the section below. This will run on the allocated node, and in this example it runs on milan2.

module load gcc

make clean
make

#rm -rf "./data/parallel"
mkdir -p "./data/parallel"

stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 100 | tee ./data/parallel/spread_100.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 200 | tee ./data/parallel/spread_200.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 300 | tee ./data/parallel/spread_300.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 400 | tee ./data/parallel/spread_400.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 500 | tee ./data/parallel/spread_500.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 600 | tee ./data/parallel/spread_600.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 700 | tee ./data/parallel/spread_700.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 800 | tee ./data/parallel/spread_800.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 900 | tee ./data/parallel/spread_900.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1000 | tee ./data/parallel/spread_1000.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1100 | tee ./data/parallel/spread_1100.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1200 | tee ./data/parallel/spread_1200.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1300 | tee ./data/parallel/spread_1300.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1400 | tee ./data/parallel/spread_1400.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1500 | tee ./data/parallel/spread_1500.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1600 | tee ./data/parallel/spread_1600.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1700 | tee ./data/parallel/spread_1700.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1800 | tee ./data/parallel/spread_1800.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1900 | tee ./data/parallel/spread_1900.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 2000 | tee ./data/parallel/spread_2000.txt

stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 16 | tee ./data/parallel/spread_16.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 32 | tee ./data/parallel/spread_32.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 64 | tee ./data/parallel/spread_64.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 128 | tee ./data/parallel/spread_128.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 256 | tee ./data/parallel/spread_256.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 512 | tee ./data/parallel/spread_512.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 1024 | tee ./data/parallel/spread_1024.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 2048 | tee ./data/parallel/spread_2048.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 4096 | tee ./data/parallel/spread_4096.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 8192 | tee ./data/parallel/spread_8192.txt
stdbuf --output=L ./assignment_ikj_tiled_parallel_spread.out 52 16384 | tee ./data/parallel/spread_16384.txt
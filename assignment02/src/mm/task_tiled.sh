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

rm -rf "./data/tiled"
mkdir -p "./data/tiled"

stdbuf --output=L ./assignment_ikj_tiled.out 1 | tee ./data/tiled/tiled_1.txt
stdbuf --output=L ./assignment_ikj_tiled.out 2 | tee ./data/tiled/tiled_2.txt
stdbuf --output=L ./assignment_ikj_tiled.out 3 | tee ./data/tiled/tiled_3.txt
stdbuf --output=L ./assignment_ikj_tiled.out 4 | tee ./data/tiled/tiled_4.txt
stdbuf --output=L ./assignment_ikj_tiled.out 5 | tee ./data/tiled/tiled_5.txt
stdbuf --output=L ./assignment_ikj_tiled.out 8 | tee ./data/tiled/tiled_8.txt
stdbuf --output=L ./assignment_ikj_tiled.out 10 | tee ./data/tiled/tiled_10.txt
stdbuf --output=L ./assignment_ikj_tiled.out 12 | tee ./data/tiled/tiled_12.txt
stdbuf --output=L ./assignment_ikj_tiled.out 14 | tee ./data/tiled/tiled_14.txt
stdbuf --output=L ./assignment_ikj_tiled.out 16 | tee ./data/tiled/tiled_16.txt
stdbuf --output=L ./assignment_ikj_tiled.out 20 | tee ./data/tiled/tiled_20.txt
stdbuf --output=L ./assignment_ikj_tiled.out 25 | tee ./data/tiled/tiled_25.txt
stdbuf --output=L ./assignment_ikj_tiled.out 30 | tee ./data/tiled/tiled_30.txt
stdbuf --output=L ./assignment_ikj_tiled.out 35 | tee ./data/tiled/tiled_35.txt
stdbuf --output=L ./assignment_ikj_tiled.out 40 | tee ./data/tiled/tiled_40.txt
stdbuf --output=L ./assignment_ikj_tiled.out 45 | tee ./data/tiled/tiled_45.txt
stdbuf --output=L ./assignment_ikj_tiled.out 50 | tee ./data/tiled/tiled_50.txt
stdbuf --output=L ./assignment_ikj_tiled.out 48 | tee ./data/tiled/tiled_48.txt
stdbuf --output=L ./assignment_ikj_tiled.out 52 | tee ./data/tiled/tiled_52.txt
stdbuf --output=L ./assignment_ikj_tiled.out 54 | tee ./data/tiled/tiled_54.txt
stdbuf --output=L ./assignment_ikj_tiled.out 46 | tee ./data/tiled/tiled_46.txt
stdbuf --output=L ./assignment_ikj_tiled.out 55 | tee ./data/tiled/tiled_55.txt
stdbuf --output=L ./assignment_ikj_tiled.out 60 | tee ./data/tiled/tiled_60.txt
stdbuf --output=L ./assignment_ikj_tiled.out 65 | tee ./data/tiled/tiled_65.txt

stdbuf --output=L ./assignment_ikj_tiled.out 300 | tee ./data/tiled/tiled_300.txt
stdbuf --output=L ./assignment_ikj_tiled.out 400 | tee ./data/tiled/tiled_400.txt
stdbuf --output=L ./assignment_ikj_tiled.out 500 | tee ./data/tiled/tiled_500.txt
stdbuf --output=L ./assignment_ikj_tiled.out 600 | tee ./data/tiled/tiled_600.txt
stdbuf --output=L ./assignment_ikj_tiled.out 700 | tee ./data/tiled/tiled_700.txt
stdbuf --output=L ./assignment_ikj_tiled.out 800 | tee ./data/tiled/tiled_800.txt
stdbuf --output=L ./assignment_ikj_tiled.out 900 | tee ./data/tiled/tiled_900.txt
stdbuf --output=L ./assignment_ikj_tiled.out 1000 | tee ./data/tiled/tiled_1000.txt
stdbuf --output=L ./assignment_ikj_tiled.out 1200 | tee ./data/tiled/tiled_1200.txt
stdbuf --output=L ./assignment_ikj_tiled.out 1500 | tee ./data/tiled/tiled_1500.txt
stdbuf --output=L ./assignment_ikj_tiled.out 70 | tee ./data/tiled/tiled_70.txt
stdbuf --output=L ./assignment_ikj_tiled.out 80 | tee ./data/tiled/tiled_80.txt
stdbuf --output=L ./assignment_ikj_tiled.out 100 | tee ./data/tiled/tiled_100.txt
stdbuf --output=L ./assignment_ikj_tiled.out 150 | tee ./data/tiled/tiled_150.txt
stdbuf --output=L ./assignment_ikj_tiled.out 200 | tee ./data/tiled/tiled_200.txt
stdbuf --output=L ./assignment_ikj_tiled.out 1 | tee ./data/tiled/tiled_1.txt
stdbuf --output=L ./assignment_ikj_tiled.out 5 | tee ./data/tiled/tiled_5.txt
stdbuf --output=L ./assignment_ikj_tiled.out 10 | tee ./data/tiled/tiled_10.txt
stdbuf --output=L ./assignment_ikj_tiled.out 30 | tee ./data/tiled/tiled_30.txt
stdbuf --output=L ./assignment_ikj_tiled.out 40 | tee ./data/tiled/tiled_40.txt
stdbuf --output=L ./assignment_ikj_tiled.out 3 | tee ./data/tiled/tiled_3.txt
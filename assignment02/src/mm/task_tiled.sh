#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J task_tiled
#Output and error (also --output, --error):
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#Initial working directory (also --chdir):
#SBATCH -D ./
#Notification and type
#SBATCH --time=00:15:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=cs #milan sap gh cs
#SBATCH --nodelist=cs1 #sap4 gh3 cs7
#SBATCH --nodes=1

# embed your test codes in the section below. This will run on the allocated node, and in this example it runs on milan2.

module load gcc

make clean
make

rm -rf "./data/tiled"
mkdir -p "./data/tiled"

stdbuf --output=L ./assignment_ikj_tiled.out 10 | tee ./data/tiled/tiled_10.txt
stdbuf --output=L ./assignment_ikj_tiled.out 20 | tee ./data/tiled/tiled_20.txt
stdbuf --output=L ./assignment_ikj_tiled.out 48 | tee ./data/tiled/tiled_48.txt
stdbuf --output=L ./assignment_ikj_tiled.out 50 | tee ./data/tiled/tiled_50.txt
stdbuf --output=L ./assignment_ikj_tiled.out 52 | tee ./data/tiled/tiled_52.txt

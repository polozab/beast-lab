#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J t2
#Output and error (also --output, --error):
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#Initial working directory (also --chdir):
#SBATCH -D ./
#Notification and type
#SBATCH --time=00:30:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=milan #milan sap gh cs
#SBATCH --nodelist=milan1 #sap4 gh3 cs7
#SBATCH --nodes=1

make clean
make NUM_TEAMS=256 NUM_THREADS=256

rm -rf "./data/t2"
mkdir -p "./data/t2"

stdbuf --output=L ./mm-gpu-offload_explicit.out | tee ./data/t2/explicit_row.csv
stdbuf --output=L ./mm-gpu-offload_implicit.out | tee ./data/t2/implicit_row.csv
stdbuf --output=L ./mm-gpu-offload_explicit_col.out | tee ./data/t2/explicit_col.csv
stdbuf --output=L ./mm-gpu-offload_implicit_col.out | tee ./data/t2/implicit_col.csv

for ((t=8; t<=70; t+=2)); do
  make clean
  make NUM_TEAMS=256 NUM_THREADS=256 TILE_SIZE=$t
  stdbuf --output=L ./mm-gpu-offload_explicit_tiling.out | tee ./data/t2/tiling_row_$t.csv
  stdbuf --output=L ./mm-gpu-offload_explicit_tiling_col.out | tee ./data/t2/tiling_col_$t.csv
done

#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J task3
#Output and error (also --output, --error):
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#Initial working directory (also --chdir):
#SBATCH -D ./
#Notification and type
#SBATCH --time=00:05:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=milan #milan sap gh cs
#SBATCH --nodelist=milan2 #sap4 gh3 cs7
#SBATCH --nodes=1

# embed your test codes in the section below. This will run on the allocated node, and in this example it runs on milan2.

#for t in 1 2 4 8 16 24 32 48 56 64 96 128 192; do
#  printf -v t_padded '%03d' "$t"
#  echo "Executing with ${t_padded} threads..."
#  export OMP_NUM_THREADS=$t
#  ../triad/triad.bin 26 1 > "task3a_triad_${t_padded}.csv"
#done

echo "Executing strong scale test..."
export OMP_NUM_THREADS=
export OMP_PROC_BIND="spread"
../triad/triad_mod.bin 29 1 > "task3b_triad_mod_spread.csv"

echo "Executing strong scale test with close thread pinning..."
export OMP_NUM_THREADS=
export OMP_PROC_BIND="close"
../triad/triad_mod.bin 29 1 > "task3b_triad_mod_close.csv"

echo "Executing strong scale test with sequential initialization..."
export OMP_NUM_THREADS=
export OMP_PROC_BIND="spread"
../triad/triad_mod_seq-init.bin 29 1 > "task3b_triad_mod_seq-init.csv"

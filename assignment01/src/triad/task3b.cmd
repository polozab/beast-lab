#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J task3b
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
#SBATCH --partition=sap #milan sap gh cs
#SBATCH --nodelist=sap3 #sap4 gh3 cs7
#SBATCH --nodes=1

# embed your test codes in the section below. This will run on the allocated node, and in this example it runs on milan2.

make clean
make

rm -rf "./data/task3b"
mkdir -p "./data/task3b"

echo "Executing strong scale test..."
export OMP_NUM_THREADS=192
export OMP_PROC_BIND="spread"
./triad_mod.bin 29 1 > "./data/task3b/triad_mod_spread.csv"

echo "Executing strong scale test with close thread pinning..."
export OMP_NUM_THREADS=192
export OMP_PROC_BIND="close"
./triad_mod.bin 29 1 > "./data/task3b/triad_mod_close.csv"

echo "Executing strong scale test with sequential initialization..."
export OMP_NUM_THREADS=192
export OMP_PROC_BIND="spread"
./triad_mod_seq-init.bin 29 1 > "./data/task3b/triad_mod_seq-init.csv"


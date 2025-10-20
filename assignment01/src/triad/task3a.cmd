#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J task3a
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

module load gcc/13.2.0-gcc-7.5.0-kv6jgtu

make clean
make

rm -rf "./data/task3a"
mkdir -p "./data/task3a"

for t in 1 16 24 32 48 64 96 128 192; do
  printf -v t_padded '%03d' "$t"
  echo "Executing with ${t_padded} threads..."
  export OMP_NUM_THREADS=$t
  ./triad.bin 26 1 > "./data/task3a/triad_${t_padded}.csv"
done


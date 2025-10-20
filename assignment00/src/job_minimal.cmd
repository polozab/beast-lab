#!/bin/bash
# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)
#SBATCH -J test
#Output and error (also --output, --error):
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#Initial working directory (also --chdir):
#SBATCH -D ./Group-101/assignment01/src/triad
#Notification and type
#SBATCH --time=00:10:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=sap #milan sap gh cs
#SBATCH --nodelist=sap3 #sap4 gh3 cs7
#SBATCH --nodes=1

# embed your test codes in the section below. This will run on the allocated node, and in this example it runs on milan2.
hostname
lscpu
echo "Set gcc 14 and params\n"
module load gcc/14.2.0-gcc-13.2.0-57z6vik
export OMP_NUM_THREADS=1 OMP_PROC_BIND=spread
echo "Test 1 default gcc\n"
make CC=gcc CFLAGS = "-fopenmp" -B
./triad.c 26 > test1.csv

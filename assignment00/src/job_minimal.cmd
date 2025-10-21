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
#SBATCH --time=00:05:00
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
module load gcc/14.2.0-gcc-13.2.0-57z6vik
export OMP_NUM_THREADS=1 OMP_PROC_BIND=spread
make CC=gcc CFLAGS="-fopenmp" -B
./triad  26 > test1.csv

echo "test 2 gcc -O2\n"
module load gcc/14.2.0-gcc-13.2.0-57z6vik
export OMP_NUM_THREADS=1 OMP_PROC_BIND=spread
make CC=gcc CFLAGS="-fopenmp -O2" -B
./triad  26 > test2.csv

echo "test 3 gcc -Ofast\n"
module load gcc/14.2.0-gcc-13.2.0-57z6vik
export OMP_NUM_THREADS=1 OMP_PROC_BIND=spread
make CC=gcc CFLAGS="-fopenmp -Ofast -march=native -mprefer-vector-width=512" -B
./triad  26 > test3.csv

echo "---\n"

echo "Test 4 default gcc\n"
module load gcc/14.2.0-gcc-13.2.0-57z6vik
export OMP_NUM_THREADS=1 OMP_PROC_BIND=spread
make CC=gcc CFLAGS="-fopenmp" -B
./triad  26 false > test4.csv

echo "test 5 gcc -O2\n"
module load gcc/14.2.0-gcc-13.2.0-57z6vik
export OMP_NUM_THREADS=1 OMP_PROC_BIND=spread
make CC=gcc CFLAGS="-fopenmp -O2" -B
./triad  26 false > test5.csv

echo "test 6 gcc -Ofast\n"
module load gcc/14.2.0-gcc-13.2.0-57z6vik
export OMP_NUM_THREADS=1 OMP_PROC_BIND=spread
make CC=gcc CFLAGS="-fopenmp -Ofast -march=native -mprefer-vector-width=512" -B
./triad  26 false > test6.csv

echo "---"
echo "test 7 best flags x num\n"
module load gcc/14.2.0-gcc-13.2.0-57z6vik
export OMP_NUM_THREADS=1 OMP_PROC_BIND=spread
make CC=gcc CFLAGS="-fopenmp -Ofast -march=native -mprefer-vector-width=512" -B
numactl --membind=2 ./triad  26 false > test7.csv

echo "Test 8 defautlt x num \n"
module load gcc/14.2.0-gcc-13.2.0-57z6vik
export OMP_NUM_THREADS=1 OMP_PROC_BIND=spread
make CC=gcc CFLAGS="-fopenmp" -B
numactl --membind=2 ./triad  26 > test8.csv

#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J a3t4-2
#Output and error (also --output, --error):
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#Initial working directory (also --chdir):
#SBATCH -D ./
#Notification and type
#SBATCH --time=00:45:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=cs #milan sap gh cs
#SBATCH --nodelist=cs7 #sap4 gh3 cs7
#SBATCH --nodes=1

# embed your test codes in the section below. This will run on the allocated node, and in this example it runs on milan2.
sleep 30
echo starting with idle 
date
sleep 300
echo end of idle 
date

sleep 30


echo starting of mm 
date 
cd Group-101/assignment03/src/mm/
module load gcc
module load likwid
export OMP_NUM_THREADS=128
make -B 
./mm.out 52 8192
./mm.out 52 8192

echo end of mm \n
date

sleep 30

echo start small n 
date
./mm.out 52 32

./mm.out 52 32

sleep 30

echo end small n 
date

echo starting triad
date

cd ../triad/
make -B
./triad.out 26
./triad.out 26
./triad.out 26

./triad.out 26
echo end of triad
sleep 30
date


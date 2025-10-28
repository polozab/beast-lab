#!/bin/bash

# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)

#SBATCH -J task_order
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
#SBATCH --nodelist=cs7 #sap4 gh3 cs7
#SBATCH --nodes=1

# embed your test codes in the section below. This will run on the allocated node, and in this example it runs on milan2.

module load gcc

make clean
make

rm -rf "./data/order"
mkdir -p "./data/order"

echo "starting\n"
stdbuf --output=L ./assignment_ijk.out | tee ./data/order/ijk.txt
echo "fiinished fst\n"

stdbuf --output=L ./assignment_jik.out | tee ./data/order/jik.txt
echo "fiinished snd\n"

stdbuf --output=L ./assignment_ikj.out | tee ./data/order/ikj.txt
echo "fiinished\n"


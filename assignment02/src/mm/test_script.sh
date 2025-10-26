#!/bin/bash
#SBATCH -J user08 gcc
#Output and error (also --output, --error):
#SBATCH -o ./%x.%j.out
#SBATCH -e ./%x.%j.err
#Initial working directory (also --chdir):
#SBATCH -D ./Group-101/assignment02/src/mm
#Notification and type
#SBATCH --time=00:10:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=cs #milan sap gh cs
#SBATCH --nodelist=cs2 #sap4 gh3 cs7
#SBATCH --nodes=1
make -B assignment

stdbuf --output=L ./assignment | tee ./perf_data.txt

python3 ../util/generate_chart.py

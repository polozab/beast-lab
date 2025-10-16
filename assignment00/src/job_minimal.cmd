#!/bin/bash
# submit via sbatch job_minimal.cmd
# Job Name and Files (also --job-name)
#SBATCH -J test
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
hostname
lscpu


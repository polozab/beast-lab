#!/usr/bin/bash

#SBATCH -J job_t2b
#Initial working directory (also --chdir):
#SBATCH -D ./
#Notification and type
#SBATCH --time=00:30:00
#SBATCH --no-requeue
#SBATCH --export=NONE
#SBATCH --get-user-env
#SBATCH --partition=sap #milan sap gh cs
#SBATCH --nodelist=sap3 #sap4 gh3 cs7
#SBATCH --nodes=1

module load likwid
make bench-instructions CHAINS_MAX=1
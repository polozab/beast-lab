#!/usr/bin/bash
rm -r ./data
rm *.out
sbatch ./scripts/job_t2b_cs7.sh
sbatch ./scripts/job_t2b_gh3.sh
sbatch ./scripts/job_t2b_milan1.sh
sbatch ./scripts/job_t2b_sap3.sh
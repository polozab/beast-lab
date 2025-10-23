#!/bin/bash

make -B assignment

stdbuf --output=L ./assignment | tee ./perf_data.txt

python3 ../util/generate_chart.py

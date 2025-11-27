# Latency Assignment

## 1. Compile and run

* For the part of `list`, you can simply run the test as follows, where the output will be directed into `perf_list.csv`.
```bash
./list 30 | tee perf_list.csv
```

The output in this code can be written into the file as you want. To do this, you can check the util function with the comment. Feel free to modify it for a more beautiful output format.

## 2. Plot the results

CI is not used in this assignment. Use the Python script as a guide for the result figures. Feel free to modify these scripts to plot it (in /util).

* Generating graphs for `list`,

```bash
python3 generate_chart_list.py --performance-data reference_output_list.csv --output-file perf_ref_list.pdf
```

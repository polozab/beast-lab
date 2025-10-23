# Assignment - MxM CPU

The files `assignment.cpp` is the basis for this assingment.

You can use the code and the provided shell script to automatically compile, run and create a plot of your code.

## Testing your solution

The `test_script.sh` provides a small executable that compiles, executes and then plots the results of the code in `assignment.cpp`.

The testscript uses the `stdbuf` program of the `coreutils` package to enable linebuffering, so the results show up once they are available instead of buffering all lines until the results get piped to `tee`.
If you can't install the `coreutils` package or it isn't available on your OS, just remove the `stdbuf --output=L ` command from the shell script.

## Recent Performance Data

![CI Performance Artifact](../-/jobs/artifacts/main/raw/assignment2/src/perf_data.svg?job=Assignment2)


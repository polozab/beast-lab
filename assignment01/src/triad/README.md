# Compile
```sh
make CC=<COMPILER> CFLAGS="<FLAGS>"
```

# Run
```sh
OMP_NUM_THREADS=<threads> OMP_PROC_BIND={spread|close} OMP_PLACES={cores|threads|sockets} ./triad <max. vector length>
```

# Example

A simple example on the Icelake system is the following:

```sh
make CC=cc CFLAGS="-fopenmp -O3"
export OMP_NUM_THREADS=2 OMP_PROC_BIND=spread OMP_PLACES=cores
./triad 25 > triad.csv
```

With this configuration, we use the default compiler (`cc`) for building and compile with the flags `-fopenmp -O3`. We use 2 threads and specify `spread` as binding and which results in running 1 thread on each socket on a system with multiple sockets, each pinned to a core (`cores`). The maximum vector length is set to 2<sup>25</sup> and the output is redirected to the file *triad.csv*.

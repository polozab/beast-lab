CC ?= gcc
CFLAGS ?= -O2 -fopenmp

all: c2c c2c_firsttouch c2c_parallel_naive

c2c: c2c.c
	$(CC) $(CFLAGS) $< -o $@

c2c_firsttouch: c2c_firsttouch.c
	$(CC) $(CFLAGS) $< -o $@

c2c_parallel_naive: c2c_parallel_naive.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f c2c c2c_firsttouch c2c_parallel_naive perf_*

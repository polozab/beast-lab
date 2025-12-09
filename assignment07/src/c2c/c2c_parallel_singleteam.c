#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <stdatomic.h>

// Parallelizes the (t1, t2) sweep using a single OpenMP team reused for all pairs.
// Only threads t1 and t2 participate in each ping-pong; others idle at barriers.
// This avoids nested teams/oversubscription but still may alter timing due to
// barriers and idle threads.

double wtime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + 1e-6*tv.tv_usec;
}

static inline void bench_body(int t1, int t2, int pings, _Atomic int *a, int tid) {
    if (tid == t1) {
        for(int c = 0; c < pings; c++) {
            atomic_store_explicit(a, 1, memory_order_release);
            while(atomic_load_explicit(a, memory_order_acquire) == 1);
        }
        atomic_store_explicit(a, 2, memory_order_release);
    } else if (tid == t2) {
        while(atomic_load_explicit(a, memory_order_acquire) != 2) {
            while(atomic_load_explicit(a, memory_order_acquire) == 0);
            if (atomic_load_explicit(a, memory_order_acquire) == 1)
                atomic_store_explicit(a, 0, memory_order_release);
        }
    }
}

int main(int argc, char* argv[])
{
    int pings = -1;
    int addrs = 0;
    int verb  = 0;
    
    if (argc > 1) pings = atoi(argv[1]);
    if (argc > 2) addrs = atoi(argv[2]);
    if (argc > 3) verb = atoi(argv[3]);

    if (pings == 0) {
        printf( "Usage: %s [<kpps> [<addrs> [<verbose>]]]\n", argv[0]);
        exit(1);
    }

    if (pings == -1) pings = 10;
    if (addrs == 0)  addrs = 30; 
    pings = pings * 1000;
    
    int *buf = malloc(addrs * 64 + 64);
    int elems = (addrs * 64 + 64) / sizeof(int);
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < elems; i++)
        buf[i] = 0;

    int tc = 0;
    #pragma omp parallel
    if (omp_get_thread_num() == 0)
        tc = omp_get_num_threads();

    printf("Running %.1f K ping-pongs using %d addresses for each pair from %d threads (single-team sweep)\n", (double) pings / 1000.0, addrs, tc);
    printf("Threads: %d, Base addr: %p\n", tc, buf);

    double *min_arr = malloc(tc*tc * sizeof(double));
    double *max_arr = malloc(tc*tc * sizeof(double));
    double *avg_arr = malloc(tc*tc * sizeof(double));

    #pragma omp parallel shared(min_arr, max_arr, avg_arr, buf, pings, addrs, verb, tc)
    {
        int tid = omp_get_thread_num();
        _Atomic int *a = (_Atomic int *)buf;
        #pragma omp barrier

        for (int t1 = 0; t1 < tc; t1++) {
            for (int t2 = 0; t2 < tc; t2++) {
                if (t1 == t2) {
                    if (tid == 0) {
                        min_arr[t1*tc + t2] = 0.0;
                        max_arr[t1*tc + t2] = 0.0;
                        avg_arr[t1*tc + t2] = 0.0;
                    }
                    #pragma omp barrier
                    continue;
                }

                // announce pair once
                if (tid == 0)
                    printf("   Pair (%2d,%2d):\n", t1, t2);

                // Warmup
                if (tid == t1 || tid == t2) {
                    double tsum = 0.0;
                    double t = wtime();
                    while(tsum < .02) {
                        bench_body(t1, t2, pings, a, tid);
                        double tt = wtime();
                        double tdiff = tt - t;
                        t = tt;
                        tsum += tdiff;
                        if (verb && tid == t2)
                            printf("    Pair (%2d,%2d) warm: %.1f ns\n", t1, t2, tdiff / 2.0 / pings * 1e9);
                    }
                }
                #pragma omp barrier

                // Measure (only t2 records stats)
                if (tid == t1 || tid == t2) {
                    double min = 0.0, max = 0.0, avg = 0.0;
                    for(int i = 0; i < addrs; i++) {
                        double tt = wtime();
                        bench_body(t1, t2, pings, a + 4*i, tid);
                        tt = (wtime() - tt) / 2.0 / pings * 1e9;

                        if (tid == t2) {
                            if (i == 0) {
                                min = max = avg = tt;
                            } else {
                                avg += tt;
                                if (tt < min) min = tt;
                                if (tt > max) max = tt;
                            }
                            if (verb)
                                printf("    Pair (%2d,%2d) Addr %p: %.1f ns\n", t1, t2, a + 4*i, tt);
                        }
                    }
                    if (tid == t2) {
                        avg = avg / addrs;
                        min_arr[t1*tc + t2] = min;
                        max_arr[t1*tc + t2] = max;
                        avg_arr[t1*tc + t2] = avg;
                    }
                }
                #pragma omp barrier
            }
        }
    }

    free(buf);
    free(min_arr);
    free(max_arr);
    free(avg_arr);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <stdatomic.h>

// Naive parallelized version: we parallelize the (t1,t2) sweep with collapse(2).
// This will spawn nested teams when bench() is called and is expected to distort
// the measurements, but it demonstrates the pitfall requested in the assignment.

void out2file(char *filename, double *val_arr, int N){
    FILE *fptr;
    fptr = fopen(filename, "w");
    
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            fprintf(fptr, "%7.3f", val_arr[i*N + j]);
            if (j != N-1){
                fprintf(fptr, ",");
            }
        }
        fprintf(fptr, "\n");
    }
    fclose(fptr);
}

double wtime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + 1e-6*tv.tv_usec;
}

void bench(int tc, int t1, int t2, int pings, int* addr)
{
    _Atomic int *a = (_Atomic int *)addr;
    atomic_store(a, 0);

    #pragma omp parallel for
    for(int i = 0; i < tc; i++) {
        if (i == t1) {
            for(int c = 0; c < pings; c++) {
                atomic_store_explicit(a, 1, memory_order_release);
                while(atomic_load_explicit(a, memory_order_acquire) == 1);
            }
            atomic_store_explicit(a, 2, memory_order_release);
        }
        else if (i == t2) {
            while(atomic_load_explicit(a, memory_order_acquire) != 2) {
                while(atomic_load_explicit(a, memory_order_acquire) == 0);
                if (atomic_load_explicit(a, memory_order_acquire) == 1)
                    atomic_store_explicit(a, 0, memory_order_release);
            }
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
        printf( "Usage: %s [<kpps> [<addrs> [<verbose>]]]\n"
            "\n"
            "Parameters:\n"
            "  <kpps>  kilo ping-pongs to do in one measure (def: 100)\n"
            "  <addrs> number of addresses to run through per core pair (def: 50)\n"
            "  <verb>  verbosity level (def: 0), if 1 show all measurements\n", argv[0]);
        exit(1);
    }

    if (pings == -1) pings = 10;
    if (addrs == 0)  addrs = 30; 
    pings = pings * 1000;
    
    int *a = malloc(addrs * 64 + 64);
    // simple first-touch to spread pages somewhat
    int elems = (addrs * 64 + 64) / sizeof(int);
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < elems; i++)
        a[i] = 0;

    int tc = 0;
    #pragma omp parallel
    if (omp_get_thread_num() == 0)
        tc = omp_get_num_threads();

    printf("Running %.1f K ping-pongs using %d different addresses for each pair from %d threads (naive parallel sweep)\n", (double) pings / 1000.0, addrs, tc);
    printf("Threads: %d, Base addr: %p\n", tc, a);

    double *min_arr = malloc(tc*tc * sizeof(double));
    double *max_arr = malloc(tc*tc * sizeof(double));
    double *avg_arr = malloc(tc*tc * sizeof(double));

    // Naively parallelize the pair sweep. This is expected to oversubscribe and
    // distort timing because bench() also creates a team.
    #pragma omp parallel for collapse(2) schedule(static)
    for(int t1 = 0; t1 < tc; t1++) {
        for(int t2 = 0; t2 < tc; t2++) {
            if (t1 == t2) {
                min_arr[t1*tc + t2] = 0.0;
                max_arr[t1*tc + t2] = 0.0;
                avg_arr[t1*tc + t2] = 0.0;
                continue;
            }

            double tsum = 0.0;
            double t = wtime();
            while(tsum < .02) {
                bench(tc, t1, t2, pings, a);
                double tt = wtime();
                double tdiff = tt - t;
                t = tt;
                tsum += tdiff;
                if (verb)
                    printf("    Pair (%2d,%2d) warm: %.1f ns\n", t1, t2, tdiff / 2.0 / pings * 1e9);
            }

            double min, max, avg;
            for(int i = 0; i < addrs; i++) {
                double tt = wtime();
                bench(tc, t1, t2, pings, a + 4*i);
                tt = (wtime() - tt) / 2.0 / pings * 1e9;
                
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
            avg = avg / addrs;

            min_arr[t1*tc + t2] = min;
            max_arr[t1*tc + t2] = max;
            avg_arr[t1*tc + t2] = avg;
        }
    }

    free(a);
    free(min_arr);
    free(max_arr);
    free(avg_arr);

    return 0;
}

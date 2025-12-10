#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <stdatomic.h>

// -------------------------------------------------------------
// Util functions
// -------------------------------------------------------------

void out2file(char *filename, double *val_arr, int N){
    FILE *fptr;
    fptr = fopen(filename, "w");
    
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            // printf("%7.3f", val_arr[i*N + j]);
            fprintf(fptr, "%7.3f", val_arr[i*N + j]);
            if (j != N-1){
                // printf(",");
                fprintf(fptr, ",");
            }
        }
        // printf("\n");
        fprintf(fptr, "\n");
    }
    fclose(fptr);
}

double wtime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + 1e-6*tv.tv_usec;
}

// Reuse a single OpenMP team for all ping-pong runs to avoid the heavy
// thread creation overhead that happens when spawning a new team per bench.
// All threads participate and synchronize via barriers; only t1 and t2
// actually exchange the cache line.
void bench_single_team(int tc, int t1, int t2, int pings, _Atomic int *addr)
{
    int tid = omp_get_thread_num();

    if (tid == 0)
        atomic_store(addr, 0);

    #pragma omp barrier

    if (tid == t1) {
        for(int c = 0; c < pings; c++) {
            atomic_store_explicit(addr, 1, memory_order_release);
            while(atomic_load_explicit(addr, memory_order_acquire) == 1);
        }
        atomic_store_explicit(addr, 2, memory_order_release);
    } else if (tid == t2) {
        while(atomic_load_explicit(addr, memory_order_acquire) != 2) {
            while(atomic_load_explicit(addr, memory_order_acquire) == 0);
            if (atomic_load_explicit(addr, memory_order_acquire) == 1)
                atomic_store_explicit(addr, 0, memory_order_release);
        }
    }

    #pragma omp barrier
}

int main(int argc, char* argv[])
{
	// parse arguments: c2c [<kilo ping-pongs> [<addresses> [<verbose>]]]
	// defaults:
	// - pings = 100 (measure 100K ping-pongs per core pair)
	// - addrs = 50 (check 50 different addresses per core pair)
	// - verb  = 0 (only print min/avg/max stat per core pair)
	//           if set to 1: show warm up for 1st addrs + per address latency
	
	int pings = -1;
	int addrs = 0;
	int verb  = 0;
	
	if (argc > 1) pings = atoi(argv[1]);
	if (argc > 2) addrs = atoi(argv[2]);
	if (argc > 3) verb = atoi(argv[3]);

	// print help if 1st arg given and cannot be parsed as int
	if (pings == 0) {
		printf( "Usage: %s [<kpps> [<addrs> [<verbose>]]]\n"
			"\n"
			"Parameters:\n"
			"  <kpps>  kilo ping-pongs to do in one measure (def: 100)\n"
			"  <addrs> number of addresses to run through per core pair (def: 50)\n"
			"  <verb>  verbosity level (def: 0), if 1 show all measurements\n", argv[0]);
		exit(1);
	}

	// set defaults if not given
	if (pings == -1) pings = 10;
	if (addrs == 0)  addrs = 30; 
	pings = pings * 1000;
    
    int *a = malloc(addrs * 64 + 64);

	// get number of threads to run ping-pongs on
	int tc = 0;
    #pragma omp parallel
    if (omp_get_thread_num() == 0)
        tc = omp_get_num_threads();

    if (tc == 0) {
        fprintf(stderr, "Failed to detect OpenMP threads\n");
        exit(1);
    }

    // for recording the result matrices
    double *min_arr = malloc(tc*tc * sizeof(double));
    double *max_arr = malloc(tc*tc * sizeof(double));
    double *avg_arr = malloc(tc*tc * sizeof(double));

    // Shared temporaries reused per pair and guarded by single sections
    double min = 0.0, max = 0.0, avg = 0.0;
    double t = 0.0, tsum = 0.0, start = 0.0;
    int continue_warmup = 1;

    #pragma omp parallel num_threads(tc) shared(tc, pings, addrs, verb, a, min_arr, max_arr, avg_arr, min, max, avg, t, tsum, start, continue_warmup)
    {
        #pragma omp single
        {
            printf("Running %.1f K ping-pongs using %d different addresses for each pair from %d threads\n", (double) pings / 1000.0, addrs, tc);
            printf("Threads: %d, Base addr: %p\n", tc, a);
        }
        #pragma omp barrier

        for(int t1 = 0; t1 < tc; t1++) {
            for(int t2 = 0; t2 < tc; t2++) {
                #pragma omp single
                {
                    if (t1 == t2) {
                        min_arr[t1*tc + t2] = 0.0;
                        max_arr[t1*tc + t2] = 0.0;
                        avg_arr[t1*tc + t2] = 0.0;
                    } else {
                        printf("   Pair (%2d,%2d):\n", t1, t2);
                    }
                    tsum = 0.0;
                    t = wtime();
                }
                #pragma omp barrier

                if (t1 == t2)
                    continue;

                // Warm up of at most 20ms for core pair
                while (1) {
                    bench_single_team(tc, t1, t2, pings, (_Atomic int *)a);

                    #pragma omp single
                    {
                        double tt = wtime();
                        double tdiff = tt - t; // elapsed time of 1 bench run
                        t = tt;
                        tsum += tdiff;
                        continue_warmup = (tsum < .02);
                        if (verb)
                            printf("    Addr %p, warm: %.1f ns\n", a, tdiff / 2.0 / pings * 1e9);
                    }
                    #pragma omp barrier

                    if (!continue_warmup)
                        break;
                }

                // Measure
                #pragma omp single
                {
                    min = max = avg = 0.0;
                }
                #pragma omp barrier

                for(int i = 0; i < addrs; i++) {
                    #pragma omp single
                    {
                        start = wtime();
                    }
                    #pragma omp barrier

                    // different addresses within different cache lines
                    bench_single_team(tc, t1, t2, pings, (_Atomic int *)(a + 4*i));

                    #pragma omp single
                    {
                        double tt = (wtime() - start) / 2.0 / pings * 1e9;

                        if (i == 0) {
                            min = max = avg = tt;
                        } else {
                            avg += tt;
                            if (tt < min) min = tt;
                            if (tt > max) max = tt;
                        }
                        if (verb)
                            printf("    Addr %p: %.1f ns\n", a + 4*i, tt);
                    }
                    #pragma omp barrier
                }

                #pragma omp single
                {
                    avg = avg / addrs;
                    printf("    Min: %.1f ns (max: %.1f ns, avg: %.1f ns)\n", min, max, avg);

                    // recording results
                    min_arr[t1*tc + t2] = min;
                    max_arr[t1*tc + t2] = max;
                    avg_arr[t1*tc + t2] = avg;
                }
                #pragma omp barrier
            }
        }
    }

    // write output file
    printf("------------------------------------\n");
    char filename[] = "./avg_matrix.csv";
    out2file(filename, avg_arr, tc);
    printf("------------------------------------\n");

    // free memory
    free(a);
    free(min_arr);
    free(max_arr);
    free(avg_arr);

	return 0;
}

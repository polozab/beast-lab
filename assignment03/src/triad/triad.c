#include <assert.h>
#include <errno.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <likwid-marker.h>

// pointers for vectors
static double *restrict A, *restrict B, *restrict C, *restrict D;

double triad(int N, size_t REP)
{
    double begin = omp_get_wtime();
#pragma omp parallel
    {
        for (size_t r = 0; r < REP; ++r)
#pragma omp for schedule(static) nowait
            for (int i = 0; i < N; ++i)
                A[i] = B[i] + C[i] * D[i];
    }
    return omp_get_wtime() - begin;
}

double triadReal(int N, size_t REP)
{

    #pragma omp parallel
    {
        LIKWID_MARKER_THREADINIT;
        LIKWID_MARKER_REGISTER("REGION");
    }

    double begin = omp_get_wtime();
    #pragma omp parallel
    {
        LIKWID_MARKER_START("REGION");

        for (size_t r = 0; r < REP; ++r)
            #pragma omp for schedule(static) nowait
            for (int i = 0; i < N; ++i)
                A[i] = B[i] + C[i] * D[i];

        LIKWID_MARKER_STOP("REGION");
    }
    
    double end = omp_get_wtime();
    return end - begin;

}

bool verify(int N)
{
    for (int i = 0; i < N; ++i) {
        if (A[i] != 7.0)
            return false;
    }
    return true;
}

void initialize_vectors(int N)
{
#pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        A[i] = 0.0;
        B[i] = 1.0;
        C[i] = 2.0;
        D[i] = 3.0;
    }
}

void allocate_vectors(size_t N, bool aligned)
{
    size_t CLS = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

    if (aligned) {
        A = aligned_alloc(CLS, N * sizeof(double));
        B = aligned_alloc(CLS, N * sizeof(double));
        C = aligned_alloc(CLS, N * sizeof(double));
        D = aligned_alloc(CLS, N * sizeof(double));
    } else {
        A = malloc(N * sizeof(double));
        B = malloc(N * sizeof(double));
        C = malloc(N * sizeof(double));
        D = malloc(N * sizeof(double));
    }

    if(!A || !B || !C || !D) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

void free_vectors(void)
{
    free(A);
    free(B);
    free(C);
    free(D);
}

int main(int argc, char *argv[])
{
    size_t N_min = 1ULL << 7;
    size_t N_max = 0U;

    // min. measurement time in seconds
    double min_duration = 1.0;
    bool   aligned      = true;
    long   num_threads  = omp_get_num_threads();

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <N> [Threads]\n", argv[0]);
        fprintf(stderr, "    N         : vector length (2^N_max)\n");
        fprintf(stderr, "    Threads   : number of threads to be used (default number of cores)\n");
        exit(EXIT_FAILURE);
    }

    char *pEnd;
    long N = 1ULL << strtol(argv[1], &pEnd, 10);
    if (errno || N > __INT_MAX__) {
        fprintf(stderr, "N invalid / out of range\n");
        exit(EXIT_FAILURE);
    }

    num_threads = strtol(argv[2], &pEnd, 10);
    if (errno || N > __INT_MAX__) {
        fprintf(stderr, "Threads invalid / out of range\n");
        exit(EXIT_FAILURE);
    }

    LIKWID_MARKER_INIT;

    printf("%s,%s,%s,%s\n", "N", "GFLOPS", "Repetitions", "Threads");

    allocate_vectors(N, aligned);
    initialize_vectors(N);
    omp_set_num_threads(num_threads);

    // warm-up and set number of repetitions
    size_t rep = 1U;
    while (triad(N, rep) < min_duration && rep < __SIZE_MAX__ / 2) {
        rep *= 2;
    }

    // actual measurement
    double time = triadReal(N, rep);

    assert(verify(N));
    free_vectors();

    // flop calculation
    double num_flops = 2.0 * N * rep;
    double gflops    = (num_flops / time) * 1.0e-9;

    printf("%zu,%.2f,%zu,%d\n", N, gflops, rep, num_threads);
    
    LIKWID_MARKER_CLOSE;

    return EXIT_SUCCESS;
}

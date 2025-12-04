#ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#endif

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct entry {
    double v;
    volatile uint64_t next;
};

const double golden_ratio = 1.61803398874989484820458683436;

double wtime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)(ts.tv_sec * 1000000000LL + ts.tv_nsec) * 1.e-9;
}

bool is_coprime(uint64_t a, uint64_t b)
{
    for (uint64_t i = 2; i <= a / 2 + 1; i++)
        if ((a % i) == 0 && (b % i) == 0)
            return false;
    return true;
}

bool check_sum(uint64_t N, double sum)
{
    double sum_expected = (double)((N - 1) * (N / 2)); // "kleiner Gauss"
    return fabs(sum - sum_expected) < 1.e-3;
}

void init(uint64_t N, uint64_t k, struct entry *A)
{
    uint64_t mask = N - 1; // N is power-of-2

    /* initialize elements ("next" will be overwritten in loop below) */
    for (uint64_t i = 0; i < N; ++i) {
        A[i].v = 0;
        A[i].next = 0;
    }

    /* initialize "next" for specific traversal */
    for (uint64_t i = 0; i < N; ++i) {
        uint64_t idx = (k * i) & mask;
        assert(A[idx].next == 0); // not over-written yet
        A[idx].next = (k * (i + 1)) & mask;
    }

    /* set A[i].v to some value != 0 using index_load traversal */
    uint64_t next = 0;
    for (uint64_t i = 0; i < N; ++i) {
        A[next].v = (double) i;
        next      = A[next].next;
    }

    /* validate all list element values are set */
    for (uint64_t i = 1; i < N; ++i) {
        assert(A[i].v != 0.0);
    }
}

double sum_indexcalc(uint64_t N, uint64_t k, uint64_t REP, double *psum, uint64_t *pdummy)
{
    double   sum   = 0.0;
    uint64_t dummy = 0;
    uint64_t mask  = N - 1; // N is power-of-2

    struct entry *A = malloc((size_t)N * sizeof(struct entry));
    if (!A) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    init(N, k, A);

    double t0 = wtime();
    for (uint64_t r = 0; r < REP; ++r) {
        sum           = 0.0;
        uint64_t next = 0;
        for (uint64_t i = 0; i < N; ++i) {
            sum   += A[next].v;
            dummy |= A[next].next;
            next   = (k * (i + 1)) & mask;
            fprintf(stderr, "%d", next);
        }
    }
    double t1 = wtime();

    *psum   = sum;
    *pdummy = dummy;

    free(A);
    return t1 - t0;
}

double sum_indexload(uint64_t N, uint64_t k, uint64_t REP, double *psum, uint64_t *pdummy)
{
    double   sum   = 0.0;
    uint64_t dummy = 0;
    uint64_t mask  = N - 1; // N is power-of-2

    struct entry *A = malloc((size_t)N * sizeof(struct entry));
    if (!A) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    init(N, k, A);

    double t0 = wtime();
    for (uint64_t r = 0; r < REP; ++r) {
        sum           = 0.0;
        uint64_t next = 0;
        for (uint64_t i = 0; i < N; ++i) {
            sum   += A[next].v;
            dummy |= (k * (i + 1)) & mask;
            next   = A[next].next;
            fprintf(stderr, "%d", next);
        }
    }
    double t1 = wtime();

    *psum   = sum;
    *pdummy = dummy;

    free(A);
    return t1 - t0;
}

// Bonus question
double sum_indexload2(uint64_t N, uint64_t k, uint64_t REP, double *psum, uint64_t *pdummy)
{
    double   sum   = 0.0;
    uint64_t dummy = 0;
    uint64_t mask  = N - 1; // N is power-of-2

    struct entry *A = malloc((size_t)N * sizeof(struct entry));
    if (!A) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    init(N, k, A);

    double t0 = wtime();
    for (uint64_t r = 0; r < REP; ++r) {
        sum           = 0.0;
        uint64_t next = 0;
        for (uint64_t i = 0; i < N; ++i) {
            sum   += A[next].v;
            dummy |= (k * (i + 1)) & mask;
            // speculate on next value
            if (next + 1 == A[next].next)
                next = next + 1;
            else
                next = A[next].next;
        }
    }
    double t1 = wtime();

    *psum   = sum;
    *pdummy = dummy;

    free(A);
    return t1 - t0;
}


int main(int argc, char **argv)
{
    uint64_t       N_max;
    const uint64_t N_min        = 1024; // 2^10
    const double   min_duration = 1.0;  // min. measurement time in seconds

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        fprintf(stderr, "    N: maximum list length (2^N_max)\n");
        exit(EXIT_FAILURE);
    }

    char *p_end;
    N_max = 1ULL << strtol(argv[1], &p_end, 10);
    if (errno || N_max > __INT64_MAX__ || N_max < N_min) {
        fprintf(stderr, "N invalid / out of range\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Linked list benchmark with max. list length of N=%" PRIu64 "\n\n", N_max);

    printf("%20s,%10s,%10s\n", "SumType", "N", "Latency[ns]");

    double   sum_calc;
    double   sum_load;
    uint64_t dummy_calc;
    uint64_t dummy_load;

#define CSV_FORMAT "%s k=%-4s,%10" PRIu64 ",%11.6f\n"

#define BENCH(FN, N, K, SUM, DUMMY)                                                                \
    do {                                                                                           \
        uint64_t rep = 1;                                                                          \
        double   time;                                                                             \
        /* warm-up and set number of repetitions */                                                \
        while ((time = FN((N), (K), rep, (SUM), (DUMMY))) < min_duration &&                        \
               rep < __UINT64_MAX__ / 2) {                                                         \
            rep *= 2;                                                                              \
        }                                                                                          \
        time                    = FN((N), (K), rep, (SUM), (DUMMY));                               \
        double time_per_element = time / ((double)(N) * (double)rep) * 1.e9;                       \
        printf(CSV_FORMAT, #FN, ((K) == 1) ? "1" : "gold", (N), time_per_element);                 \
    } while (0)

    for (uint64_t N = N_min; N <= N_max; N *= 2) {

        BENCH(sum_indexcalc, N, 1, &sum_calc, &dummy_calc);
        BENCH(sum_indexload, N, 1, &sum_load, &dummy_load);
        //BENCH(sum_indexload2, N, 1, &sum_load, &dummy_load);

        assert(dummy_calc == dummy_load);
        assert(check_sum(N, sum_calc));
        assert(check_sum(N, sum_load));

        /*
         * Ensure N and k are coprime and set k such that the ratio of
         * N and k is close to golden ratio for pseudo-randomness
         */
        uint64_t k = (uint64_t)((double)N / golden_ratio);
        k          = (k % 2 == 0) ? k - 1 : k;
        assert(is_coprime(N, k));

        BENCH(sum_indexcalc, N, k, &sum_calc, &dummy_calc);
        BENCH(sum_indexload, N, k, &sum_load, &dummy_load);

        assert(dummy_calc == dummy_load);
        assert(check_sum(N, sum_calc));
        assert(check_sum(N, sum_load));
    }
    return EXIT_SUCCESS;
}

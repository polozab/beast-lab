// Fast(er) simple version: column-major C, one-time mapping/init on device,
// reuse data for all repetitions to avoid re-copies.

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <omp.h>

#define IDX(row, col, width) ((width) * (row) + (col))
#define IDX_COL(row, col, height) ((row) + (col) * (height))

static void mm_kernel(double *a, double *b, double *c, int N)
{
    #pragma omp target teams loop collapse(2)
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            double sum = 0.0;
            for (int k = 0; k < N; ++k)
            {
                sum += b[IDX(i, k, N)] * c[IDX_COL(j, k, N)];
            }
            a[IDX(i, j, N)] += sum;
        }
    }
}

double mm(int N, int REP, double expected_result)
{
    const long N_sqr = N * N;
    double *a = (double *)malloc(N_sqr * sizeof(double));
    double *b = (double *)malloc(N_sqr * sizeof(double));
    double *c = (double *)malloc(N_sqr * sizeof(double));
    if (!a || !b || !c) { fprintf(stderr, "alloc failed\n"); exit(EXIT_FAILURE); }

    // Host init
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            a[IDX(i, j, N)] = 0.0;
            b[IDX(i, j, N)] = (i * j) / (N * 50);
            c[IDX_COL(i, j, N)] = (i * j) / (N * 50);
        }
    }

    double dur;

    // Map once, reuse inside REP loop
    #pragma omp target enter data map(to: b[0:N_sqr], c[0:N_sqr]) map(alloc: a[0:N_sqr])

    // Zero A on device
    #pragma omp target teams loop collapse(2) map(present, tofrom: a[0:N_sqr])
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            a[IDX(i, j, N)] = 0.0;

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int r = 0; r < REP; ++r)
    {
        mm_kernel(a, b, c, N);
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    #pragma omp target exit data map(from: a[0:N_sqr]) map(delete: b[0:N_sqr], c[0:N_sqr])

    using dsec = std::chrono::duration<double>;
    dur = std::chrono::duration_cast<dsec>(t1 - t0).count();

    // Verify
    double sum = 0.0;
    for (long i = 0; i < N_sqr; ++i) sum += a[i];
    if (std::abs(sum - expected_result) > 0.001)
    {
        printf("Wrong result for N=%4d. expected %.3f but got %.3f. Aborting...\n", N, expected_result, sum);
        exit(EXIT_FAILURE);
    }

    free(a); free(b); free(c);

    double mflop = 2.0 * (double)N * (double)N * (double)N * (double)REP * 1.0e-6 / dur;
    printf("%d, %d, %f\n", N, REP, mflop);
    return mflop;
}

int main()
{
    printf("N, REP, MFLOPs\n");
    mm(100, 2000, 108290000.000);
    mm(200, 250, 1170799500.000);
    mm(300, 75, 3560702175.000);
    mm(400, 32, 7353887776.000);
    mm(500, 16, 12171228800.000);
    mm(750, 10, 64265028760.000);
    mm(1000, 6, 171249832524.000);
    mm(1250, 4, 359491225496.000);
    mm(1500, 3, 685019722065.000);
    mm(1750, 2, 1001806087176.000);
    mm(2000, 2, 1974958963390.000);
    mm(2500, 1, 3060585851209.000);
    mm(3000, 1, 7694687665084.000);
    mm(3500, 1, 16754058063962.000);
    mm(4000, 1, 32845267978538.000);
}

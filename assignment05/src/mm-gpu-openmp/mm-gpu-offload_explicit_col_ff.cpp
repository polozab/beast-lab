#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <omp.h>

#ifndef NUM_TEAMS
#define NUM_TEAMS 64
#endif

#ifndef NUM_THREADS
#define NUM_THREADS 64
#endif

#define IDX(row, col, width) ((width) * (row) + (col))
#define IDX_COL(row, col, height) ((row) + (col) * (height))

static void mm_kernel(double *a, double *b, double *c, int N)
{
    #pragma omp target is_device_ptr(a, b, c)
    #pragma omp teams num_teams(NUM_TEAMS)
    #pragma omp distribute parallel for num_threads(NUM_THREADS) collapse(2) schedule(dynamic, 1)
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
    const long N_sqr = (long)N * (long)N;
    double *a = (double *)malloc(N_sqr * sizeof(double));
    double *b = (double *)malloc(N_sqr * sizeof(double));
    double *c = (double *)malloc(N_sqr * sizeof(double));
    if (!a || !b || !c)
    {
        fprintf(stderr, "alloc failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            a[IDX(i, j, N)] = 0.0;
            b[IDX(i, j, N)] = (i * j) / (N * 50);
            c[IDX_COL(i, j, N)] = (i * j) / (N * 50);
        }
    }

    int actual_teams = -1;
    int actual_threads = -1;

    #pragma omp target enter data map(to: b[0:N_sqr], c[0:N_sqr]) map(alloc: a[0:N_sqr])

    #pragma omp target is_device_ptr(a)
    #pragma omp teams num_teams(NUM_TEAMS)
    #pragma omp distribute parallel for num_threads(NUM_THREADS) collapse(2) schedule(static, 1)
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            a[IDX(i, j, N)] = 0.0;
        }
    }

    #ifdef PRINT_DEBUG
    #pragma omp target map(from: actual_teams, actual_threads)
    #pragma omp teams num_teams(NUM_TEAMS)
    #pragma omp distribute parallel for num_threads(NUM_THREADS)
    for (int i = 0; i < 1; ++i)
    {
        if (omp_get_team_num() == 0 && omp_get_thread_num() == 0)
        {
            actual_teams = omp_get_num_teams();
            actual_threads = omp_get_num_threads();
        }
    }
    #endif

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int r = 0; r < REP; ++r)
    {
        mm_kernel(a, b, c, N);
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    #pragma omp target exit data map(from: a[0:N_sqr]) map(delete: b[0:N_sqr], c[0:N_sqr])

    using dsec = std::chrono::duration<double>;
    double dur = std::chrono::duration_cast<dsec>(t1 - t0).count();

    double sum = 0.0;
    for (long i = 0; i < N_sqr; ++i)
    {
        sum += a[i];
    }
    if (std::abs(sum - expected_result) > 0.001)
    {
        printf("Wrong result for N=%4d. expected %.3f but got %.3f. Aborting...\n",
               N, expected_result, sum);
        exit(EXIT_FAILURE);
    }

    free(a);
    free(b);
    free(c);

    double mflop = 2.0 * (double)N * (double)N * (double)N * (double)REP * 1.0e-6 / dur;
    printf("%d, %d, %d, %d, %d, %d, %f\n",
           N, REP, NUM_TEAMS, NUM_THREADS, actual_teams, actual_threads, mflop);
    return mflop;
}

int main()
{
    printf("N, REP, Teams, Threads, Actual_Teams, Actual_Threads, MFLOPS\n");
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

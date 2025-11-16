#include <chrono>
#include <cstdio>
#include <math.h>
#include <omp.h>

#ifndef NUM_TEAMS
#define NUM_TEAMS 64
#endif

#ifndef NUM_THREADS
#define NUM_THREADS 64
#endif

#ifndef TILE_SIZE
#define TILE_SIZE 1024
#endif

#define IDX(row, col, width) ((width) * (row) + (col))

void mm_kernel(double *a, double *b, double *c, int N, int padded_N)
{
    #pragma omp target
    #pragma omp teams num_teams(NUM_TEAMS)
    #pragma omp distribute parallel for num_threads(NUM_THREADS) collapse(2) schedule(dynamic, 1)
    for (int i = 0; i < N; i += TILE_SIZE)
    {
        for (int j = 0; j < N; j += TILE_SIZE)
        {
            for (int k = 0; k < N; k += TILE_SIZE)
            {
                for (int ii = 0; ii < TILE_SIZE; ++ii)
                {
                    for (int jj = 0; jj < TILE_SIZE; ++jj)
                    {
                        for (int kk = 0; kk < TILE_SIZE; ++kk)
                        {
                            a[IDX(i + ii, j + jj, padded_N)] += b[IDX(i + ii, k + kk, padded_N)] * c[IDX(j + jj, k + kk,  padded_N)];
                        }
                    }
                }
            }
        }
    }

}

double mm(int N, int REP, double expected_result)
{
    int padded_N = N;
    if (N > TILE_SIZE) {
        padded_N = ((N + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE;
    }

    const long padded_N_sqr = padded_N*padded_N;
    double dur;

    // Alloc for CPU ref only
    double *a = (double *)malloc(padded_N_sqr * sizeof(double));
    double *b = (double *)malloc(padded_N_sqr * sizeof(double));
    double *c = (double *)malloc(padded_N_sqr * sizeof(double));

    int *teams = (int *) omp_target_alloc(sizeof(int), omp_get_default_device());
    int *threads = (int *) omp_target_alloc(sizeof(int), omp_get_default_device());

    *teams = -1;
    *threads = -1;

    printf("%d\n", padded_N);

    // Alloc on GPU
    #pragma omp target data map(from : a[0 : padded_N_sqr]) map(alloc : b[0 : padded_N_sqr], c[0 : padded_N_sqr]) 
    {

        // Init data on cpu
        #pragma omp target
        #pragma omp teams num_teams(NUM_TEAMS)
        #pragma omp distribute parallel for num_threads(NUM_THREADS) collapse(2) schedule(static, 1)
        for (int i = 0; i < padded_N; ++i)
        {
            for (int j = 0; j < padded_N; ++j)
            {
                if (i < N && j < N) {
                    a[IDX(i, j, padded_N)] = 0.0;
                    b[IDX(i, j, padded_N)] = (i * j) / (N * 50);
                    c[IDX(i, j, padded_N)] = (i * j) / (N * 50);
                }
                else {
                    a[IDX(i, j, padded_N)] = 0.0;
                    b[IDX(i, j, padded_N)] = 0.0;
                    c[IDX(i, j, padded_N)] = 0.0;
                }
            }
        }

        // Debug print 
        #ifdef PRINT_DEBUG
        #pragma omp target
        #pragma omp teams num_teams(NUM_TEAMS)
        #pragma omp distribute parallel for num_threads(NUM_THREADS) collapse(2) schedule(static, 1)
        for (int i = 0; i < padded_N; ++i)
        {
            for (int j = 0; j < padded_N; ++j)
            {
                *teams = omp_get_num_teams();
                *threads = omp_get_num_threads();
            }
        }
        printf("TEST\n");
        #endif

        // Start time measurements for computation
        auto t0 = std::chrono::high_resolution_clock::now();

        // TODO: Run on target device
        for (int r = 0; r < REP; ++r)
        {
            mm_kernel(a, b, c, N, padded_N);
        }

        // End time measurements for computation
        auto t1 = std::chrono::high_resolution_clock::now();

        // Time-measured calculation
        using dsec = std::chrono::duration<double>;
        dur = std::chrono::duration_cast<dsec>(t1 - t0).count();

    }

    // TODO: Copy back from GPU, eventually release memory

    // Simple correctness check
    double sum = 0.0;
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; j++) {

            sum += a[IDX(i, j, padded_N)];
        }
    }

    // Verify expected result, accounting for possible system dependent rounding errors
    if (std::abs(sum - expected_result) > 0.001)
    {
        printf("Wrong result for N=%4d. expected %.3f but got %.3f. Aborting...\n",
            N,
            expected_result,
            sum);
        exit(EXIT_FAILURE);
    }

    // Free memory
    free((void *)a);
    free((void *)b);
    free((void *)c);
    omp_target_free(teams, omp_get_default_device());
    omp_target_free(threads, omp_get_default_device());

    // Flop calculation
    double mflop = 2.0 * (double)padded_N * (double)padded_N * (double)padded_N * (double)REP * 1.0e-6 / dur;
    printf("%d, %d, %d, %d, %d, %d, %d, %d, %f\n", N, padded_N, TILE_SIZE, REP, NUM_TEAMS, NUM_THREADS, *teams, *threads, mflop);

    return mflop;

}

int main(int argc, char *argv[])
{

    (void)argc;
    (void)argv;

    // example code for checking num threads, num teams
    // #pragma omp target
    // #pragma omp teams num_teams(numTeams) thread_limit(numThreads)
    // #pragma omp parallel num_threads(numThreads)
    // {
    //   printf("Team [%d/%d]: Thread [%d/%d] ...\n", omp_get_team_num(),
    //   omp_get_num_teams(), omp_get_thread_num(), omp_get_num_threads());
    // }
    // printf("--------------------------------------------------\n");

    printf("N, Padded_N, Tiles, REP, Teams, Threads, Actual_Teams, Actual_Threads, MFLOPS\n");

    // ugly hardcoded stuff :)
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

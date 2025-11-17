// Multi-GPU implicit version (column-major C) with on-device initialization of A slices.
// Each GPU gets its slice of A allocated/zeroed on device, and B/C copied once.

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <omp.h>

#define IDX(row, col, width) ((width) * (row) + (col))
#define IDX_COL(row, col, height) ((row) + (col) * (height))

// Kernel for a slice starting at i0 (global rows) of length rows.
static void mm_kernel_slice(double *a_slice, double *b, double *c, int N, int i0, int rows, int dev)
{
    #pragma omp target teams loop collapse(2) device(dev) \
        map(present, tofrom: a_slice[0 : rows * N]) \
        map(present, to: b[0 : N * N]) \
        map(present, to: c[0 : N * N])
    for (int ii = 0; ii < rows; ++ii)
    {
        for (int j = 0; j < N; ++j)
        {
            for (int k = 0; k < N; ++k)
            {
                a_slice[IDX(ii, j, N)] += b[IDX(i0 + ii, k, N)] * c[IDX_COL(j, k, N)];
            }
        }
    }
}

double mm(int N, int REP, double expected_result)
{
    const long N_sqr = N * N;
    double dur;

    double *a = (double *)malloc(N_sqr * sizeof(double));
    double *b = (double *)malloc(N_sqr * sizeof(double));
    double *c = (double *)malloc(N_sqr * sizeof(double));

    if (!a || !b || !c)
    {
        fprintf(stderr, "Allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Init on host for B/C. A will be zeroed on device.
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            b[IDX(i, j, N)] = (i * j) / (N * 50);
            c[IDX_COL(i, j, N)] = (i * j) / (N * 50);
            a[IDX(i, j, N)] = 0.0;
        }
    }

    int num_dev = omp_get_num_devices();
    if (num_dev < 1)
        num_dev = 1;

    auto t0 = std::chrono::high_resolution_clock::now();

    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            // Stage data on each device: copy B/C, allocate A slice, zero it.
            for (int dev = 0; dev < num_dev; ++dev)
            {
                int rows = N / num_dev;
                int i0 = dev * rows;
                if (dev == num_dev - 1)
                {
                    rows = N - i0;
                }

                #pragma omp task firstprivate(dev, rows, i0)
                {
                    // Copy B, C; allocate A slice.
                    #pragma omp target enter data device(dev) \
                        map(to: b[0 : N_sqr], c[0 : N_sqr]) \
                        map(alloc: a[i0 * N : rows * N])

                    // Zero A slice on device.
                    #pragma omp target teams loop device(dev) \
                        map(present, tofrom: a[i0 * N : rows * N])
                    for (int idx = 0; idx < rows * N; ++idx)
                    {
                        a[i0 * N + idx] = 0.0;
                    }
                }
            }
            #pragma omp taskwait

            // Compute
            for (int r = 0; r < REP; ++r)
            {
                for (int dev = 0; dev < num_dev; ++dev)
                {
                    int rows = N / num_dev;
                    int i0 = dev * rows;
                    if (dev == num_dev - 1)
                    {
                        rows = N - i0;
                    }

                    #pragma omp task firstprivate(dev, rows, i0)
                    {
                        mm_kernel_slice(a + i0 * N, b, c, N, i0, rows, dev);
                    }
                }
                #pragma omp taskwait
            }

            // Copy back A slices and release device data.
            for (int dev = 0; dev < num_dev; ++dev)
            {
                int rows = N / num_dev;
                int i0 = dev * rows;
                if (dev == num_dev - 1)
                {
                    rows = N - i0;
                }

                #pragma omp task firstprivate(dev, rows, i0)
                {
                    #pragma omp target exit data device(dev) \
                        map(from: a[i0 * N : rows * N]) \
                        map(delete: b[0 : N_sqr], c[0 : N_sqr]) \
                        map(delete: a[i0 * N : rows * N])
                }
            }
            #pragma omp taskwait
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    using dsec = std::chrono::duration<double>;
    dur = std::chrono::duration_cast<dsec>(t1 - t0).count();

    double sum = 0.0;
    for (long i = 0; i < N_sqr; ++i)
    {
        sum += a[i];
    }

    if (std::abs(sum - expected_result) > 0.001)
    {
        printf("Wrong result for N=%4d. expected %.3f but got %.3f. Aborting...\n",
               N,
               expected_result,
               sum);
        exit(EXIT_FAILURE);
    }

    free((void *)a);
    free((void *)b);
    free((void *)c);

    double mflop = 2.0 * (double)N * (double)N * (double)N * (double)REP * 1.0e-6 / dur;
    printf("N=%d, REP=%d, devices=%d, MFLOPs=%f\n", N, REP, num_dev, mflop);

    return mflop;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("N, REP, devices, MFLOPs\n");

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

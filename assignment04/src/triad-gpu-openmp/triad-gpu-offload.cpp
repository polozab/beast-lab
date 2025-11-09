#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <omp.h>

double calculateChecksum(long N, const double *vector)
{
    double checksum = 0.0;
    for (int i = 0; i < N; i++)
    {
        checksum += vector[i];
    }
    return checksum;
}

void triad(unsigned long N, unsigned long repetitions, long numTeams, long numThreads)
{

    auto *a = new double[N];
    auto *b = new double[N];
    auto *c = new double[N];
    auto *d = new double[N];

    // CPU = 1; GPU = 0
    int isInitialDevice = omp_is_initial_device();

    #pragma omp target map(isInitialDevice)
    {
        isInitialDevice = omp_is_initial_device();
    }

    /*
     *TODO: Variant 1 - (CPU to GPU copy after CPU initialization) is done by the for loop below.
     *TODO: Variant 2 - Implement and test
     */
    for (unsigned long i = 0; i < N; i++)
    {
        a[i] = b[i] = c[i] = d[i] = i;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // GPU offloading of the computation loops
    #pragma omp target map(a, b, c, d)
    {
        for (unsigned long j = 0; j < repetitions; ++j)
        {
            /*
             *TODO: Add OpenMP directives and clauses to fully utilize the GPU
             */

            #pragma omp loop
            for (unsigned long i = 0; i < N; ++i)
            {
                a[i] = b[i] + c[i] * d[i];
            }
        }
    } /* target end */

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start).count();

    double checksum = calculateChecksum(N, a);

    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;

    double mflops = 2.0 * N * repetitions * 1.0e-6 / duration;

    // print the experimental results
    printf("| %10ld | %8ld | %8ld | %8.2f | %8ld | %4d | %.4e |\n",
           N,
           numTeams,
           numThreads,
           mflops,
           repetitions,
           !isInitialDevice,
           checksum);
}

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        printf("The two parameters maximum dataset size and total number of processed points need to be provided.\n");
        exit(1);
    }

    char *pEnd;
    long maximumN = 1U << strtol(argv[1], &pEnd, 10);
    printf("%d\n", maximumN);
    if (errno == ERANGE)
    {
        printf("Problem with the first number.");
        exit(2);
    }
    long totalNumberProcessedPoints = strtol(argv[2], &pEnd, 10);
    if (errno == ERANGE)
    {
        printf("Problem with the second number.");
        exit(3);
    }

    int num_devices = omp_get_num_devices();
    if (num_devices <= 0)
    {
        printf("--------------------------------------\n");
        printf(" No OpenMP Offload device found\n");
        printf("--------------------------------------\n");
        exit(1);
    }

    /*
     * TODO: You can adjust thread and team numbers by passing argv[3] and argv[4]
     */
    long numThreads = 32;
    long numTeams = 64;
    if (argc >= 4)
        numTeams = strtol(argv[3], &pEnd, 10);
    if (argc == 5)
        numThreads = strtol(argv[4], &pEnd, 10);

    fprintf(stderr,
            "Maximum dataset size = %ld, total number of processed points = %ld. "
            "Performance in MFLOPS.\n",
            maximumN,
            totalNumberProcessedPoints);

    printf("| %10s | %8s | %8s | %8s | %8s | %4s | %10s |\n",
           "Data size",
           "Teams",
           "Threads",
           "MFLOPS",
           "Cycles",
           "GPU",
           "Checksum");

    long N = 64;
    while (N <= maximumN)
    {

        // Keep the total number of processed points constant by
        // adjusting the number of repetitions according to data set size
        long cycles = std::clamp(totalNumberProcessedPoints / N, 8l, 65536l);

        // Run benchmark
        triad(N, cycles, numTeams, numThreads);

        N *= 2;
    }

    return 0;
}

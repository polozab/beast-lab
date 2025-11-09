#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <omp.h>

void triad(size_t N, size_t repetitions, long numTeams, long numThreads)
{

    static double *a = (double*) malloc(sizeof(double) * N);
    static double *b = (double*) malloc(sizeof(double) * N);
    static double *c = (double*) malloc(sizeof(double) * N);
    static double *d = (double*) malloc(sizeof(double) * N);

    // CPU = 1; GPU = 0
    int isInitialDevice = omp_is_initial_device();

    #pragma omp target map(from:isInitialDevice)
    {
        isInitialDevice = omp_is_initial_device();
    }

    /*
     *TODO: Variant 1 - (CPU to GPU copy after CPU initialization) is done by the for loop below.
     *TODO: Variant 2 - Implement and test
     */
    for (size_t i = 0; i < N; i++)
    {
        a[i] = b[i] = c[i] = d[i] = i;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // GPU offloading of the computation loops
    #pragma omp target map(tofrom:a) map(to:b,c,d)
    {
        for (size_t j = 0; j < repetitions; ++j)
        {
            /*
             *TODO: Add OpenMP directives and clauses to fully utilize the GPU
             */

            #pragma omp loop
            for (size_t i = 0; i < N; ++i)
            {
                a[i] = b[i] + c[i] * d[i];
            }
        }
    } /* target end */

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start).count();

    double checksum = 0.0;
    for (size_t i = 0; i < N; i++)
    {
        checksum += a[i];
    }
    

    // Clean up & print results
    free(a);
    free(b);
    free(c);
    free(d);

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
    size_t maximumN = 1ULL << strtol(argv[1], &pEnd, 10);
    if (errno || maximumN > __INT_MAX__ || maximumN < 1ULL << 7) {
        fprintf(stderr, "N invalid / out of range\n");
        exit(EXIT_FAILURE);
    }
    size_t totalNumberProcessedPoints = strtol(argv[2], &pEnd, 10);
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

    size_t N = 1ULL << 7;
    while (N <= maximumN)
    {

        // Keep the total number of processed points constant by
        // adjusting the number of repetitions according to data set size
        long cycles = std::clamp(totalNumberProcessedPoints / N, 8ul, 65536ul);

        // Run benchmark
        triad(N, cycles, numTeams, numThreads);

        N *= 2;
    }

    return 0;
}

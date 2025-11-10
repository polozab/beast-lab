#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <omp.h>

void triad(size_t N, long repetitions, long numTeams, long numThreads)
{

    // Check if offloading is enabled
    // CPU = 1; GPU = 0

    int isInitialDevice = omp_is_initial_device();

    #pragma omp target map(from:isInitialDevice)
    {
        isInitialDevice = omp_is_initial_device();
    }

    // Allocate memory

    double *a = new double[N];
    double *b = new double[N];
    double *c = new double[N];
    double *d = new double[N];

    // Init memory

    for (size_t i = 0; i < N; i++)
    {
        a[i] = b[i] = c[i] = d[i] = i;
    }


    // Measurement
    
    double duration;

    #pragma omp target data map(tofrom:a[0:N]) map(to:b[0:N],c[0:N],d[0:N])
    {

        auto start = std::chrono::high_resolution_clock::now();

        #pragma omp target teams
        for (size_t j = 0; j < repetitions; ++j)
        {  
            #pragma omp distribute parallel for simd num_teams(numTeams) num_threads(numThreads)
            for (size_t i = 0; i < N; ++i)
            {
                a[i] = b[i] + c[i] * d[i];
            }
        }

        auto stop = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration<double>(stop - start).count();

    } /* target end */


    // Calculate checksum

    double checksum = 0.0;
    for (int i = 0; i < N; i++)
    {
        checksum += a[i];
    }

    // Clean up & print results

    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;

    double mflops = 2.0 * N * repetitions * 1.0e-6 / duration;

    // print the experimental results
    printf("%10ld, %8ld, %8ld, %8.2f, %8ld, %4d, %.4e\n",
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

    if (argc < 2)
    {
        printf("Vector length len = 2^N needs to be provided.\nUsage: %s <len> [teams] [threads]\n", argv[0]);
        exit(1);
    }

    char *pEnd;
    size_t maximumN = 1ULL << strtol(argv[1], &pEnd, 10);
    if (errno || maximumN > __INT_MAX__ || maximumN < 1ULL << 7) {
        fprintf(stderr, "N invalid / out of range\n");
        exit(EXIT_FAILURE);
    }
    
    int num_devices = omp_get_num_devices();
    if (num_devices <= 0)
    {
        printf("--------------------------------------\n");
        printf(" No OpenMP Offload device found\n");
        printf("--------------------------------------\n");
        exit(1);
    }


    long numThreads = 32;
    long numTeams = 64;
    if (argc >= 3)
        numTeams = strtol(argv[2], &pEnd, 10);
    if (argc == 4)
        numThreads = strtol(argv[3], &pEnd, 10);

    fprintf(stderr, "Maximum dataset size = %ld, Performance in MFLOPS.\n", maximumN);

    printf("%10s, %8s, %8s, %8s, %8s, %4s, %10s\n",
           "Data size",
           "Teams",
           "Threads",
           "MFLOPS",
           "Cycles",
           "GPU",
           "Checksum");

    size_t N = 1ULL << 6;
    while (N <= maximumN)
    {

        // Keep the total number of processed points constant by
        // adjusting the number of repetitions according to data set size
        long cycles = std::clamp(2147483648 / N, 8ul, 65536ul);

        // Run benchmark
        triad(N, cycles, numTeams, numThreads);

        N *= 2;
    }

    return 0;
}

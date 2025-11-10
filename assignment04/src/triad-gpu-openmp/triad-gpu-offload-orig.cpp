#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <omp.h>
#include <string>
#include <cctype>

enum class SchedulePolicy
{
    Default,
    StaticChunk
};

const char *schedulePolicyToString(SchedulePolicy policy)
{
    switch (policy) {
    case SchedulePolicy::StaticChunk:
        return "static";
    case SchedulePolicy::Default:
    default:
        return "default";
    }
}

SchedulePolicy parseScheduleArgument(const char *arg)
{
    if (arg == nullptr) {
        return SchedulePolicy::StaticChunk;
    }

    std::string normalized(arg);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (normalized == "default" || normalized == "none") {
        return SchedulePolicy::Default;
    }
    if (normalized == "static" || normalized == "static_chunk") {
        return SchedulePolicy::StaticChunk;
    }

    fprintf(stderr, "Unknown scheduling policy \"%s\". Falling back to static scheduling.\n", arg);
    return SchedulePolicy::StaticChunk;
}

double calculateChecksum(long N, const double *vector)
{
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += vector[i];
    }
    return checksum;
}

void triad(unsigned long N,
           unsigned long repetitions,
           long          numTeams,
           long          numThreads,
           SchedulePolicy schedulePolicy)
{

    auto *a = new double[N];
    auto *b = new double[N];
    auto *c = new double[N];
    auto *d = new double[N];

    // Device Ids CPU = 1 , GPU = 0
    int targetDeviceId = 1;

/*
 *TODO: Check by below code block to see if you can offload to GPU properly
 */
#pragma omp target map(tofrom : targetDeviceId, numTeams, numThreads)
    {
        targetDeviceId = omp_is_initial_device();
    }
    printf("--------------------------------------\n");
    printf("Device ID: %d", targetDeviceId);
    printf("--------------------------------------\n");

    /*
     *TODO: Variant 1 - (CPU to GPU copy after CPU initialization) is done by the for loop below.
     *TODO: Variant 2 - Implement and test
     */
    for (unsigned long i = 0; i < N; i++) {
        a[i] = b[i] = c[i] = d[i] = i;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Keep vectors on the device for all repetitions
#pragma omp target enter data map(alloc : a[0 : N]) map(to : b[0 : N], c[0 : N], d[0 : N])
    for (unsigned long j = 0; j < repetitions; ++j) {
        if (schedulePolicy == SchedulePolicy::StaticChunk) {
            #pragma omp target teams distribute parallel for simd schedule(static) \
                num_teams(numTeams) thread_limit(numThreads) \
                map(present : a[0 : N], b[0 : N], c[0 : N], d[0 : N])
            for (unsigned long i = 0; i < N; ++i) {
                a[i] = b[i] + c[i] * d[i];
            }
        } else {
            #pragma omp target teams distribute parallel for simd num_teams(numTeams) \
                thread_limit(numThreads) map(present : a[0 : N], b[0 : N], c[0 : N], d[0 : N])
            for (unsigned long i = 0; i < N; ++i) {
                a[i] = b[i] + c[i] * d[i];
            }
        }
    }
#pragma omp target exit data map(from : a[0 : N]) map(delete : b[0 : N], c[0 : N], d[0 : N])

    auto stop     = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start).count();

    double checksum = calculateChecksum(N, a);

    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;

    double mflops = 2.0 * N * repetitions * 1.0e-6 / duration;

    // print the experimental results
    printf("| %10ld | %8ld | %8ld | %8.2f | %8ld | %4d | %8s | %.4e |\n",
           N,
           numTeams,
           numThreads,
           mflops,
           repetitions,
           targetDeviceId,
           schedulePolicyToString(schedulePolicy),
           checksum);
}

int main(int argc, char *argv[])
{

    if (argc < 3) {
        printf("The two parameters maximum dataset size and total number of processed "
               "points need to be provided.\n");
        exit(1);
    }

    char *pEnd;
    long  maximumN = strtol(argv[1], &pEnd, 10);
    if (errno == ERANGE) {
        printf("Problem with the first number.");
        exit(2);
    }
    long totalNumberProcessedPoints = strtol(argv[2], &pEnd, 10);
    if (errno == ERANGE) {
        printf("Problem with the second number.");
        exit(3);
    }

    int num_devices = omp_get_num_devices();
    if (num_devices <= 0) {
        printf("--------------------------------------\n");
        printf(" No OpenMP Offload device found\n");
        printf("--------------------------------------\n");
        exit(1);
    }

    /*
     * TODO: You can adjust thread and team numbers by passing argv[3] and argv[4]
     */
    long           numThreads     = 32;
    long           numTeams       = 64;
    SchedulePolicy schedulePolicy = SchedulePolicy::StaticChunk;
    if (argc >= 4)
        numTeams = strtol(argv[3], &pEnd, 10);
    if (argc >= 5)
        numThreads = strtol(argv[4], &pEnd, 10);
    if (argc >= 6)
        schedulePolicy = parseScheduleArgument(argv[5]);

    fprintf(stderr,
            "Maximum dataset size = %ld, total number of processed points = %ld. "
            "Performance in MFLOPS.\n",
            maximumN,
            totalNumberProcessedPoints);

    printf("| %10s | %8s | %8s | %8s | %8s | %4s | %8s | %10s |\n",
           "Data size",
           "Teams",
           "Threads",
           "MFLOPS",
           "Cycles",
           "GPU",
           "Sched",
           "Checksum");

    long N = 64;
    while (N <= maximumN) {

        // Keep the total number of processed points constant by
        // adjusting the number of repetitions according to data set size
        long cycles = std::clamp(totalNumberProcessedPoints / N, 8l, 65536l);

        // Run benchmark
        triad(N, cycles, numTeams, numThreads, schedulePolicy);

        N *= 2;
    }

    return 0;
}

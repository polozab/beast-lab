#include <likwid-marker.h>

static double *restrict A, *restrict B, *restrict C, *restrict D;

int main(int argc, char *argv[])
{

  long N = 1L >> 20;

  LIKWID_MARKER_INIT;

  #pragma omp parallel
  {
      LIKWID_MARKER_THREADINIT;
      LIKWID_MARKER_REGISTER("REGION");
  }

  #pragma omp parallel
  {
      LIKWID_MARKER_START("REGION");

      #pragma omp for schedule(static) nowait
      for (int i = 0; i < N; ++i)
        A[i] = B[i] + C[i] * D[i];

      LIKWID_MARKER_STOP("REGION");
  }

  LIKWID_MARKER_CLOSE;

  int res = 0;

  for (int i = 0; i < N; ++i)
    res += A[i];

  return res;

}
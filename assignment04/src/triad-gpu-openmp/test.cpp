#include <stdio.h>
#include <omp.h>

int main(int argc, char const *argv[])
{
  int init = omp_is_initial_device();
  int dev = omp_get_device_num();
  printf("Device Num: %d (%d)\n", dev, init);

  #pragma omp target
  {
    // int init = omp_is_initial_device();
    int dev = omp_get_device_num();
    printf("Device Num: %d (%d)\n", dev, init);
  }

  return 0;
}

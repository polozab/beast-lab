#include <stdio.h>
#include <omp.h>

int main(int argc, char const *argv[])
{
  int init = omp_is_initial_device();
  int dev = omp_get_device_num();
  printf("Device Num: %d (%d)\n", dev, init);

  #pragma omp target map(init, dev)
  {
    dev = omp_get_device_num();
    init = omp_is_initial_device();
  }
  
  printf("Inital? %d \n", init);

  return 0;
}

#include <stdio.h>
#include <omp.h>

int main(int argc, char const *argv[])
{
  int init = omp_is_initial_device();
  int dev = omp_get_device_num();
  printf("Device Num: %d (%d)\n", dev, init);

  printf("Total devices: %d\n", omp_get_num_devices());

  #pragma omp target map(init, dev)
  {
    init = omp_is_initial_device();
  }
  
  printf("Inital? %d \n", init);

  return 0;
}

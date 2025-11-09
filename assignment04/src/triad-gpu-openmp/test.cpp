#include <stdio.h>
#include <omp.h>

int main(int argc, char const *argv[])
{
  int init = omp_is_initial_device();
  int dev = omp_get_device_num();
  printf("Device Num: %d (%d)\n", dev, init);

  int a = -1;

  #pragma omp target map()
  {
    a = 2;
  
  printf("Inital? %d \n", a);

  return 0;
}

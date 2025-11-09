#include <stdio.h>
#include <omp.h>

int main(int argc, char const *argv[])
{
  int init = omp_is_initial_device();
  int dev = omp_get_device_num();
  printf("Device Num: %d (%d)\n", dev, init);

  int a = 0;

  #pragma omp target
  {
    a = 3;
  }
  
  printf("%d\n", a);
  printf("Device Num: %d (%d)\n", dev, init);

  return 0;
}

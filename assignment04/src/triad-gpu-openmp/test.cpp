#include <stdio.h>
#include <omp.h>

int main(int argc, char const *argv[])
{
  int dev = omp_get_device_num();
  printf("Device Num: %d\n", dev);
  return 0;
}

// put your working code in here to use the test_script.sh and CI

#include <chrono>
#include <math.h>
#include <cstdio>

// use a 1d array to guarantee a dense and sequential memory layout
#define TWO_D_ACCESS(row, col, width) ((width) * (row) + (col))

double mm(int N, int TILE_SIZE, int REP, double expected_result) {

  TILE_SIZE = std::min(N, TILE_SIZE);
  int padded_N = ((N + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE;
  printf("Overhead: %d, ", padded_N - N);

  //////////////////////////////////////////////////////////////////////////
  // ALLOCATION
  /////////////////////////////////////////////////////////////////////////

  double * a = (double *) malloc(padded_N*padded_N*sizeof(double));
  double * b = (double *) malloc(padded_N*padded_N*sizeof(double));
  double * c = (double *) malloc(padded_N*padded_N*sizeof(double));

  //////////////////////////////////////////////////////////////////////////
  // INIT
  /////////////////////////////////////////////////////////////////////////

  for (int i = 0; i < padded_N; i++)
  {
    for (int j = 0; j < padded_N; j++)
    {
      a[TWO_D_ACCESS(i, j, padded_N)] = 0.0;
      if (i<N && j<N) {
        b[TWO_D_ACCESS(i,j,padded_N)] = atan(TWO_D_ACCESS(i,j,N));
        c[TWO_D_ACCESS(i,j,padded_N)] = cos(TWO_D_ACCESS(i,j,N));
      }
      else {
        b[TWO_D_ACCESS(i,j,padded_N)] = 0.0;
        c[TWO_D_ACCESS(i,j,padded_N)] = 0.0;
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // WARM UP
  /////////////////////////////////////////////////////////////////////////
  
  for( int r=0; r<REP; ++r ) {
    for( int i=0; i<padded_N; i++ ) {
      for( int k=0; k<padded_N; k++ ) {
        for( int j=0; j<padded_N; j++ ) {
          a[TWO_D_ACCESS(i, j, padded_N)] += b[TWO_D_ACCESS(i, k, padded_N)] * c[TWO_D_ACCESS(k, j, padded_N)];
        }
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // MATRIX MULTIPLICATION KERNELS
  /////////////////////////////////////////////////////////////////////////

  auto t0 = std::chrono::high_resolution_clock::now();

  for( int r=0; r<REP; ++r ) {
    for( int i=0; i<N; i+=TILE_SIZE ) {
      for( int k=0; k<N; k+=TILE_SIZE ) {
        for( int j=0; j<N; j+=TILE_SIZE ) {
          for(int ii=0; ii<TILE_SIZE; ++ii) {
            for(int kk=0; kk<TILE_SIZE; ++kk) {
              for(int jj=0; jj<TILE_SIZE; ++jj) {
                a[TWO_D_ACCESS(i+ii, j+jj, padded_N)] += b[TWO_D_ACCESS(i+ii, k+kk, padded_N)] * c[TWO_D_ACCESS(k+kk, j+jj, padded_N)];
              }
            }
          }
        }
      }
    }
  }

  auto t1 = std::chrono::high_resolution_clock::now();

  // simple correctness check
  double array_sum = 0;
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      array_sum += a[TWO_D_ACCESS(i,j, padded_N)];
    }
  }
  
  // verify expected result. accounting for possibly system dependent float rounding errors
  if(std::abs(array_sum - expected_result) > 0.1){
    printf("Wrong result for N=%4d. expected %.3f but got %.3f. Aborting...\n", N, expected_result, array_sum);
    exit(EXIT_FAILURE);
  }
  //printf("Solution(N=%4d): %.3f\n", N, array_sum);

  free((void *) a);
  free((void *) b);
  free((void *) c);
  using dsec = std::chrono::duration<double>;
  double dur = std::chrono::duration_cast<dsec>(t1-t0).count();

  double mflop = 2.0*(double)padded_N*(double)padded_N*(double)padded_N*(double)REP*1.0e-6;
  return mflop/dur;
}

int main(int argc, char* argv[]) {

  // get tile size
  int tile_size = 10;
  if (argc > 1) {
    if (sscanf(argv[1], "%d", &tile_size) != 1) {
      return -1;
    }
  }

  // ugly hardcoded stuff
  double mf;

  mf = mm(100, tile_size, 3200, 702147.857);
  printf("MFLOPS(N=%4d): %.3f\n", 100, mf);

  mf = mm(200, tile_size, 1600, 1211484.385);
  printf("MFLOPS(N=%4d): %.3f\n", 200, mf);

  mf = mm(300, tile_size, 800, -212360.190);
  printf("MFLOPS(N=%4d): %.3f\n", 300, mf);

  mf = mm(400, tile_size, 400, -257987.574);
  printf("MFLOPS(N=%4d): %.3f\n", 400, mf);

  mf = mm(500, tile_size, 200, -115630.730);
  printf("MFLOPS(N=%4d): %.3f\n", 500, mf);

  mf = mm(600, tile_size, 100, -92905.739);
  printf("MFLOPS(N=%4d): %.3f\n", 600, mf);

  mf = mm(700, tile_size, 60, -49174.596);
  printf("MFLOPS(N=%4d): %.3f\n", 700, mf);

  mf = mm(800, tile_size, 40, 102953.727);
  printf("MFLOPS(N=%4d): %.3f\n", 800, mf);

  mf = mm(900, tile_size, 20, 55208.396);
  printf("MFLOPS(N=%4d): %.3f\n", 900, mf);

  mf = mm(1000, tile_size, 10, -9100.239);
  printf("MFLOPS(N=%4d): %.3f\n", 1000, mf);

  mf = mm(1100, tile_size, 5, 19076.488);
  printf("MFLOPS(N=%4d): %.3f\n", 1100, mf);

  mf = mm(1300, tile_size, 2, -4334.610);
  printf("MFLOPS(N=%4d): %.3f\n", 1300, mf);

  mf = mm(1500, tile_size, 1, 1062.209);
  printf("MFLOPS(N=%4d): %.3f\n", 1500, mf);

  mf = mm(1700, tile_size, 1, -2689.469);
  printf("MFLOPS(N=%4d): %.3f\n", 1700, mf);

  mf = mm(1900, tile_size, 1, 9179.534);
  printf("MFLOPS(N=%4d): %.3f\n", 1900, mf);
 
  return 0;
}

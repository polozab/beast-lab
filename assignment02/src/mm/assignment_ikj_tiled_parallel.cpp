// put your working code in here to use the test_script.sh and CI

#include <chrono>
#include <math.h>
#include <cstdio>
#include <omp.h>

// use a 1d array to guarantee a dense and sequential memory layout
#define TWO_D_ACCESS(row, col, width) ((width) * (row) + (col))

void init_vectors(double * a, double * b, double * c, int N, int padded_N) {
  for (int i = 0; i < padded_N; i++) {
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
}

struct return_info
{
  double mflops;
  int repetition;
  double array_sum;
};


return_info mm(int N, int TILE_SIZE, int padded_N) {

  //////////////////////////////////////////////////////////////////////////
  // ALLOCATION
  /////////////////////////////////////////////////////////////////////////

  double * a = (double *) malloc(padded_N*padded_N*sizeof(double));
  double * b = (double *) malloc(padded_N*padded_N*sizeof(double));
  double * c = (double *) malloc(padded_N*padded_N*sizeof(double));


  //////////////////////////////////////////////////////////////////////////
  // DETERMINE REPETITIONS
  /////////////////////////////////////////////////////////////////////////

  init_vectors(a, b, c, N, padded_N);

  int rep = 2;
  double wDur;

  while (true) {

    auto w0 = std::chrono::high_resolution_clock::now();

    for( int r=0; r<rep; ++r ) {
      for( int i=0; i<padded_N; i++ ) {
        for( int k=0; k<padded_N; k++ ) {
          for( int j=0; j<padded_N; j++ ) {
            a[TWO_D_ACCESS(i, j, padded_N)] += b[TWO_D_ACCESS(i, k, padded_N)] * c[TWO_D_ACCESS(k, j, padded_N)];
          }
        }
      }
    }

    auto w1 = std::chrono::high_resolution_clock::now();

    using dsec = std::chrono::duration<double>;
    wDur = std::chrono::duration_cast<dsec>(w1-w0).count();

    if (wDur > 1.0) break;
    rep *= 2;

  }

  //////////////////////////////////////////////////////////////////////////
  // WARM UP
  /////////////////////////////////////////////////////////////////////////
  
  #pragma omp parallel
  {
    for( int r=0; r<rep; ++r ) {

      #pragma omp for collapse(2)
      for( int i=0; i<N; i+=TILE_SIZE ) {
        for( int j=0; j<N; j+=TILE_SIZE ) {
          for( int k=0; k<N; k+=TILE_SIZE ) {
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

  }

  //////////////////////////////////////////////////////////////////////////
  // MATRIX MULTIPLICATION KERNELS
  /////////////////////////////////////////////////////////////////////////

  auto t0 = std::chrono::high_resolution_clock::now();

  #pragma omp parallel
  {
    for( int r=0; r<rep; ++r ) {

      #pragma omp for collapse(2)
      for( int i=0; i<N; i+=TILE_SIZE ) {
        for( int j=0; j<N; j+=TILE_SIZE ) {
          for( int k=0; k<N; k+=TILE_SIZE ) {
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

  free((void *) a);
  free((void *) b);
  free((void *) c);
  using dsec = std::chrono::duration<double>;
  double dur = std::chrono::duration_cast<dsec>(t1-t0).count();

  double mflop = 2.0*(double)padded_N*(double)padded_N*(double)padded_N*(double)rep*1.0e-6;

  return_info ri = {mflop / dur, rep, array_sum};

  return ri;

}

int main(int argc, char* argv[]) {

  // get optional matrix size and tile size
  int tile_size;
  int matrix_size;
  if (argc > 2) {
    if (sscanf(argv[1], "%d", &tile_size) != 1) {
      return -1;
    }
    if (sscanf(argv[2], "%d", &matrix_size) != 1) {
      return -1;
    }
  }
  else {
    printf("Usage: %s <tile_size> <matrix_size>\n", argv[0]);
    return -1;
  }

  size_t max_num_threads = omp_get_max_threads() * 16;

  printf("N, N_padded, Tiles, Threads, Repetitions, MFLOPS, Speedup\n");

  // Run baseline
  
  omp_set_num_threads(1);

  tile_size = std::min(matrix_size, tile_size);
  int padded_N = ((matrix_size + tile_size - 1) / tile_size) * tile_size;

  return_info rf = mm(matrix_size, tile_size, padded_N);
  printf("%d, %d, %d, %ld, %d, %f, %f\n", matrix_size, padded_N, tile_size, (size_t)1, rf.repetition, rf.mflops, 1.0);

  double baseline = rf.mflops;

  // Run strong scaling

  for (size_t num_threads = 2; num_threads <= max_num_threads; num_threads *= 2) {

    omp_set_num_threads(num_threads);

    tile_size = std::min(matrix_size, tile_size);
    int padded_N = ((matrix_size + tile_size - 1) / tile_size) * tile_size;

    return_info rf = mm(matrix_size, tile_size, padded_N);
    double speedup = rf.mflops / baseline;

    printf("%d, %d, %d, %ld, %d, %f, %f\n", matrix_size, padded_N, tile_size, num_threads, rf.repetition, rf.mflops, speedup);

  }
 
  return 0;
}

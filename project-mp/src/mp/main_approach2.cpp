#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>
#include <iostream>
#include <rocprofiler-sdk-roctx/roctx.h>
#include <omp.h>

using namespace std;

void precom_mu(double* ts, double* mu, int mlen, int sublen){

  double sum = 0;
  for (int i=0; i<sublen; i++){
    sum+=ts[i];
  }
  mu[0]=sum/sublen;

  for (int i=1; i<mlen; i++){
    mu[i] = mu[i-1] + ( ts[i+sublen-1]-ts[i-1] ) / sublen;
  }

  return;
}

void precom_norm(double* ts, double* norm, double* mu, int mlen, int sublen){

  double sum, tmp;
  for (int i=0; i<mlen; i++){
    sum = 0.0;
    tmp = 0.0;
    for (int j=0; j<sublen; j++){
      tmp = ts[i+j]-mu[i];
      sum+=tmp*tmp;
    }
    norm[i] = sqrt(1.0/sum);
  }

}

void precom_df(double* ts, double* df, int mlen, int sublen){

  df[0]=0;
  for (int i=0; i<mlen-1; i++){
    df[i+1] = ( ts[i+sublen]-ts[i] ) * 0.5 ;
  }

  return;
}

void precom_dg(double* ts, double* mu, double* dg, int mlen, int sublen){

  dg[0] = 0;
  for (int i=0; i<mlen-1; i++){
    dg[i+1] = ts[i+sublen] - mu[i+1] +  ts[i] - mu[i]; //TODO doublly check this
  }

  return;
}

void precom_QT(double* ts, double* mu, double* block_QT, int mlen, int sublen, int num_blocks) {

  const int rows = mlen - sublen;

  #pragma omp target teams distribute num_teams(num_blocks) is_device_ptr(block_QT)
  for (int b = 0; b < num_blocks; b++) {

    const int i0 = (rows * b) / num_blocks;

    const int offset = b * mlen;
    double *QT_local = block_QT + offset;

    for (int d = 0; d < rows - i0; ++d) {
      int j = i0 + sublen + d;
      double acc = 0.0;
      for (int t = 0; t < sublen; ++t) {
        acc += (ts[i0 + t] - mu[i0]) * (ts[j + t] - mu[j]);
      }
      QT_local[d] = acc;
    }
  }
}

void kernel(double *df, double *dg, double *norm, double *block_mp, int *block_mpi, double* block_QT, int mlen, int sublen, int num_blocks) {

  const int rows = mlen - sublen;

  #pragma omp target teams distribute num_teams(num_blocks) is_device_ptr(block_mp, block_mpi, block_QT)
  for (int b = 0; b < num_blocks; b++) {

    const int i0 = ((rows * b) / num_blocks);
    const int i1 = ((rows * (b + 1)) / num_blocks);

    const int offset = b * mlen;
    double *mp_local = block_mp + offset;
    int *mpi_local = block_mpi + offset;
    double *QT_local = block_QT + offset;

    for (int i = i0; i < i1; i++) {
      for (int j = i + sublen; j < mlen; j++) {

        const int d = j - i - sublen;
        if (i != i0) {
          QT_local[d] += df[i] * dg[j] + df[j] * dg[i];
        }

        const double cr = QT_local[d] * norm[i] * norm[j];

        if (cr > mp_local[i]) {
          mp_local[i] = cr;
          mpi_local[i] = j;
        }

        if (cr > mp_local[j]) {
          mp_local[j] = cr;
          mpi_local[j] = i;
        }
      }
    }
  }
}

void reduce_block_buffers(double *mp, int *mpi, const double *block_mp, const int *block_mpi, int mlen, int num_blocks) {
  #pragma omp target teams distribute parallel for is_device_ptr(block_mp, block_mpi)
  for (int i = 0; i < mlen; i++) {

    double best_val = -1;
    int best_idx = -1;

    for (int t = 0; t < num_blocks; t++) {
      size_t offset = (size_t)t * mlen + i;
      double v = block_mp[offset];
      if (v > best_val) { best_val = v; best_idx = block_mpi[offset]; }
    }

    mp[i]  = best_val;
    mpi[i] = best_idx;
  }
}

int main(int argc, char* argv[]){

  // argument handling
  if(argc < 5){
    printf("Usage: exe <input file> <len> <sublen> <blocks>\n");
    exit(0);
  }

  int len = 1 << atoi(argv[2]);
  FILE* f = fopen(argv[1],"r");
  int sublen = atoi(argv[3]);
  int mlen = len - sublen + 1;
  int num_blocks = atoi(argv[4]);

  if(f == NULL){
    perror("fopen");
    exit(1);
  }

  // decleration
  double *ts, *mp;
  int* mpi;
  double *mu, *df, *dg, *norm;
  double *tmp;

  // allocation
  ts = (double*) malloc(len * sizeof(double) );
  mp = (double*) malloc(mlen * sizeof(double) );
  mpi = (int*) malloc(mlen * sizeof(int) );

  mu = (double*) malloc(mlen * sizeof(double) );
  norm = (double*) malloc(mlen * sizeof(double) );
  df = (double*) malloc(mlen * sizeof(double) );
  dg = (double*) malloc(mlen * sizeof(double) );

  tmp= (double*) malloc(mlen * sizeof(double) );

  for(int i = 0; i < len; i++){
    fscanf(f, "%lf\n", ts+i);
  }
  fclose(f);

  // initialize output
  for(int i=0; i<mlen; i++){
    mp[i]=-1;
    mpi[i]=-1;
  }

  roctxRangePush("Init");
  // prepare statistics, initialization (some needed for the first row of matrix)
  precom_mu(ts,mu,mlen,sublen);
  precom_norm(ts,norm,mu,mlen,sublen);
  precom_df(ts,df,mlen,sublen);
  precom_dg(ts,mu,dg,mlen,sublen);

  roctxRangePop();

  double tb, te;
  roctxRangePush("Data");

  const int dev  = omp_get_default_device();
  const int block_mlen = num_blocks * mlen;

  double *block_mp = (double*) omp_target_alloc(block_mlen * sizeof(double), dev);
  int *block_mpi = (int*) omp_target_alloc(block_mlen * sizeof(int), dev);
  double *block_QT = (double*) omp_target_alloc(block_mlen * sizeof(double), dev);

  if (!block_mp || !block_mpi || !block_QT) {
    exit(1);
  }
  
  #pragma omp target teams distribute parallel for is_device_ptr(block_mp, block_mpi, block_QT)
  for (int idx = 0; idx < block_mlen; ++idx) {
    block_mp[idx]  = -1.0;
    block_mpi[idx] = -1;
    block_QT[idx] = 0.0;
  }

  #pragma omp target data \
  map(to : df[:mlen], dg[:mlen], norm[:mlen], ts[:len], mu[:mlen]) \
  map(tofrom : mp[:mlen], mpi[:mlen])
  {

    roctxRangePush("QT init");
    precom_QT(ts, mu, block_QT, mlen, sublen, num_blocks);
    roctxRangePop();

    roctxRangePush("Computation");
    tb = omp_get_wtime();
  
    roctxRangePush("Kernel");
    kernel(df, dg, norm, block_mp, block_mpi, block_QT, mlen, sublen, num_blocks);
    roctxRangePop();
  
    roctxRangePush("Reduction");
    reduce_block_buffers(mp, mpi, block_mp, block_mpi, mlen, num_blocks);
    roctxRangePop();
  
    te = omp_get_wtime();
    roctxRangePop();
  }

  omp_target_free(block_mp, dev);
  omp_target_free(block_mpi, dev);
  omp_target_free(block_QT, dev);

  roctxRangePop();

  const double t = te - tb;
  const uint64_t M = (uint64_t)mlen;
  const uint64_t S = (uint64_t)sublen;
  const uint64_t flops = (M - S) * (3*(M - S) - 1);
  const double gflops = (double)flops / (t * 1e9);

  printf("Runtime: %lf\n", t);
  printf("GFLOP/S: %.3f\n", gflops);

  // so far the cacluate mp is based on pearson correlation
  // we perform the following loop to convert mp to ED.
  for (int j=0; j<mlen;j++)
    mp[j] = sqrt( 2 * sublen * (1 - mp[j]) );

  // write the results
  f = fopen("output/mp.txt","w");
  for (int i=0;i<mlen;i++)
    fprintf(f, "%f\n", mp[i]);
  fclose(f);

  f = fopen("output/mpi.txt","w");
  for (int i=0;i<mlen;i++)
    fprintf(f, "%d\n", mpi[i]);
  fclose(f);

  // deallocation
  free (ts); free(mp); free(mpi); free(mu); free(norm); free(df); free(dg); free(tmp);
  return 0;
}

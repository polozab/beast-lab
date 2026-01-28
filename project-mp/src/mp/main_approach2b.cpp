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

void kernel(const double *ts, const double *mu, const double *df, const double *dg, const double *norm, double *block_mp, int *block_mpi, int mlen, int sublen, int num_teams, int num_threads, int tiles_k, int tiles_i)
{
  const int K = mlen - sublen;

  #pragma omp target teams num_teams(num_teams) thread_limit(num_threads) is_device_ptr(block_mp, block_mpi)
  {
    const int team = omp_get_team_num();

    #pragma omp distribute collapse(2)
    for (int kt = 0; kt < K; kt += tiles_k) {
      for (int it = 0; it < K; it += tiles_i) {

        #pragma omp parallel
        {

          const int thread = omp_get_thread_num();
          double *local_mp = block_mp + (team*num_threads + thread)*mlen;
          int *local_mpi = block_mpi + (team*num_threads + thread)*mlen;

          #pragma omp for
          for (int k = kt; k < min(kt+tiles_k, K); k++) {
  
            int i0 = it;
            int i1 = min(it + tiles_i, K - k);
            if (i0 >= i1) continue;
  
            int j0 = i0 + sublen + k;
  
            double QTk = 0.0;
            for (int t=0; t<sublen; t++){
              QTk += (ts[i0+t]-mu[i0]) * (ts[j0+t]-mu[j0]);
            }
  
            for (int i = i0; i < i1; i++) {
              int j = i + sublen + k;
  
              if (i != i0) 
                QTk += df[i]*dg[j] + df[j]*dg[i];
  
              double cr = QTk * norm[i] * norm[j];
  
              if (cr > local_mp[i]) {
                local_mp[i]=cr;
                local_mpi[i]=j;
              }
              if (cr > local_mp[j]) {
                local_mp[j]=cr;
                local_mpi[j]=i;
              }
  
            }
          }
        }
      }
    }
  }
}

void reduce_block_buffers(double *mp, int *mpi, const double *block_mp, const int *block_mpi, int mlen, int num_teams, int num_threads) {
  #pragma omp target teams distribute parallel for is_device_ptr(block_mp, block_mpi)
  for (int i = 0; i < mlen; i++) {

    double best_val = -1;
    int best_idx = -1;

    for (int team = 0; team < num_teams; team++) {
      for (int thread = 0; thread < num_threads; thread++) {
        int offset = (team * num_threads + thread) * mlen + i;
        double v = block_mp[offset];
        if (v > best_val) {
          best_val = v;
          best_idx = block_mpi[offset];
        }
      }
    }

    mp[i]  = best_val;
    mpi[i] = best_idx;
  }
}

int main(int argc, char* argv[]){

  // argument handling
  if(argc < 8){
    printf("Usage: exe <input file> <len> <sublen> <teams> <threads> <tiles_k> <tiles_i>\n");
    exit(0);
  }

  int len = 1 << atoi(argv[2]);
  FILE* f = fopen(argv[1],"r");
  int sublen = atoi(argv[3]);
  int mlen = len - sublen + 1;
  int num_teams = atoi(argv[4]);
  int num_threads = atoi(argv[5]);
  int tiles_k = atoi(argv[6]);
  int tiles_i = atoi(argv[7]);

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
  const int block_mlen = num_threads * num_teams * mlen;

  double *block_mp = (double*) omp_target_alloc(block_mlen * sizeof(double), dev);
  int *block_mpi = (int*) omp_target_alloc(block_mlen * sizeof(int), dev);

  if (!block_mp || !block_mpi) {
    exit(1);
  }
  
  #pragma omp target teams distribute parallel for is_device_ptr(block_mp, block_mpi)
  for (int idx = 0; idx < block_mlen; ++idx) {
    block_mp[idx]  = -1.0;
    block_mpi[idx] = -1;
  }

  #pragma omp target data \
  map(to : df[:mlen], dg[:mlen], norm[:mlen], ts[:len], mu[:mlen], mlen, sublen, num_teams, num_threads, tiles_k, tiles_i) \
  map(tofrom : mp[:mlen], mpi[:mlen])
  {

    roctxRangePush("Computation");
    tb = omp_get_wtime();
  
    roctxRangePush("Kernel");
    kernel(ts, mu, df, dg, norm, block_mp, block_mpi, mlen, sublen, num_teams, num_threads, tiles_k, tiles_i);
    roctxRangePop();
  
    roctxRangePush("Reduction");
    reduce_block_buffers(mp, mpi, block_mp, block_mpi, mlen, num_teams, num_threads);
    roctxRangePop();
  
    te = omp_get_wtime();
    roctxRangePop();
  }

  omp_target_free(block_mp, dev);
  omp_target_free(block_mpi, dev);

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

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

void kernel(double *QT, double *df, double *dg, double *norm, double *team_mp, int *team_mpi, int mlen, int sublen, int num_teams, int tile_size_k) {

  #pragma omp target teams num_teams(num_teams) is_device_ptr(team_mp, team_mpi)
  {
    const int team = omp_get_team_num();

    double *local_mp = team_mp + (size_t)team * mlen;
    int    *local_mpi = team_mpi + (size_t)team * mlen;

    #pragma omp distribute
    for (int k = 0; k < mlen - sublen; k++) {

      double QTk = QT[k];

      for (int i = 0; i < mlen - sublen - k; i++) {
        int j = i + sublen + k;

        if (i != 0) {
          QTk += df[i]*dg[j] + df[j]*dg[i];
        }

        double cr = QTk * norm[i] * norm[j];

        if (cr > local_mp[i]) { local_mp[i] = cr; local_mpi[i] = j; }
        if (cr > local_mp[j]) { local_mp[j] = cr; local_mpi[j] = i; }
      }

      QT[k] = QTk;
    }
  }
}

void reduce_team_buffers(double *mp, int *mpi, const double *team_mp, const int *team_mpi, int mlen, int num_teams) {
  #pragma omp target teams distribute parallel for is_device_ptr(team_mp, team_mpi)
  for (int i = 0; i < mlen; i++) {

    double best_val = -1;
    int best_idx = -1;

    for (int t = 0; t < num_teams; t++) {
      size_t offset = (size_t)t * mlen + i;
      double v = team_mp[offset];
      if (v > best_val) { best_val = v; best_idx = team_mpi[offset]; }
    }

    mp[i]  = best_val;
    mpi[i] = best_idx;
  }
}

int main(int argc, char* argv[]){

  // argument handling
  if(argc < 6){
    printf("Usage: exe <input file> <len> <sublen> <teams> <tile_size>\n");
    exit(0);
  }

  int len = 1 << atoi(argv[2]);
  FILE* f = fopen(argv[1],"r");
  int sublen = atoi(argv[3]);
  int mlen = len - sublen + 1;
  int num_teams = atoi(argv[4]);
  int tile_size = atoi(argv[5]);

  if(f == NULL){
    perror("fopen");
    exit(1);
  }

  // decleration
  double *ts, *mp;
  int* mpi;
  double *mu, *df, *dg, *norm;
  double *QT, *tmp;

  // allocation
  ts = (double*) malloc(len * sizeof(double) );
  mp = (double*) malloc(mlen * sizeof(double) );
  mpi = (int*) malloc(mlen * sizeof(int) );

  mu = (double*) malloc(mlen * sizeof(double) );
  norm = (double*) malloc(mlen * sizeof(double) );
  df = (double*) malloc(mlen * sizeof(double) );
  dg = (double*) malloc(mlen * sizeof(double) );

  QT = (double*) malloc(mlen * sizeof(double) );
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

  for(int i =0;i<mlen;i++){
	  QT[i]=0;
  }

  // initialization of the first row of matrix
  int start = 0;
  for(int i =0;i<mlen;i++){
    for (int j=0;j<sublen;j++){
      if (i-sublen >=0)
      QT[i-sublen]+= (ts[start + j] - mu[start] )*(ts[i+j]- mu[i]);
    }
  }

  roctxRangePop();

  double tb, te;
  roctxRangePush("Data");

  const int dev  = omp_get_default_device();
  const int team_mlen = num_teams * mlen;

  double *team_mp  = (double*) omp_target_alloc(team_mlen * sizeof(double), dev);
  int    *team_mpi = (int*)    omp_target_alloc(team_mlen * sizeof(int), dev);

  if (!team_mp || !team_mpi) {
    exit(1);
  }
  
  #pragma omp target teams distribute parallel for is_device_ptr(team_mp, team_mpi)
  for (int idx = 0; idx < team_mlen; ++idx) {
    team_mp[idx]  = -1;
    team_mpi[idx] = -1;
  }

  #pragma omp target data \
  map(to : df[:mlen], dg[:mlen], norm[:mlen], QT[:mlen]) \
  map(tofrom : mp[:mlen], mpi[:mlen])
  {
    roctxRangePush("Computation");
    tb = omp_get_wtime();
  
    roctxRangePush("Kernel");
    kernel(QT, df, dg, norm, team_mp, team_mpi, mlen, sublen, num_teams, tile_size);
    roctxRangePop();
  
    roctxRangePush("Reduction");
    reduce_team_buffers(mp, mpi, team_mp, team_mpi, mlen, num_teams);
    roctxRangePop();
  
    te = omp_get_wtime();
    roctxRangePop();
  }

  omp_target_free(team_mp, dev);
  omp_target_free(team_mpi, dev);

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
  free (ts); free(mp); free(mpi); free(mu); free(norm); free(df); free(dg); free(QT); free(tmp);
  return 0;
}

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

void kernel(double *mp, int *mpi, const double *ts, const double *mu,
            const double *df, const double *dg, const double *norm,
            int mlen, int sublen, int num_blocks,
            double *mp_block, int *mpi_block, double *QT_block) {

  const int rows = mlen - sublen;

  #pragma omp target teams distribute num_teams(num_blocks)
  for (int b = 0; b < num_blocks; b++) {

    const int i0 = ((rows * b) / num_blocks);
    const int i1 = ((rows * (b + 1)) / num_blocks);

    const int offset = b * mlen;
    double *mp_local = mp_block + offset;
    int *mpi_local = mpi_block + offset;
    double *QT_local = QT_block + offset;

    int start = i0;
    for(int i = 0; i < i1; i++){
      for (int j = 0; j < sublen; j++){
        if (i-sublen >=0)
        QT_local[i-sublen] += (ts[start + j] - mu[start] )*(ts[i+j]- mu[i]);
      }
    }

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

  #pragma omp target teams distribute parallel for
  for (int i = 0; i < mlen; i++) {
    double best = -1.0;
    int best_j = -1;
    for (int b = 0; b < num_blocks; b++) {
      const int idx = b * mlen + i;
      const double val = mp_block[idx];
      if (val > best) {
        best = val;
        best_j = mpi_block[idx];
      }
    }
    mp[i] = best;
    mpi[i] = best_j;
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
  int num_blocks = atoi(argv[4]);

  int mlen = len - sublen + 1;

  if(f == NULL){
    perror("fopen");
    exit(1);
  }

  // decleration
  double *ts, *mp;
  int* mpi;
  double *mu, *df, *dg, *norm;
  double *mp_block, *QT_block;
  int *mpi_block;

  // allocation
  ts = (double*) malloc(len * sizeof(double) );
  mp = (double*) malloc(mlen * sizeof(double) );
  mpi = (int*) malloc(mlen * sizeof(int) );

  mu = (double*) malloc(mlen * sizeof(double) );
  norm = (double*) malloc(mlen * sizeof(double) );
  df = (double*) malloc(mlen * sizeof(double) );
  dg = (double*) malloc(mlen * sizeof(double) );

  const size_t block_elems = (size_t)num_blocks * (size_t)mlen;
  mp_block = (double*) malloc(block_elems * sizeof(double));
  mpi_block = (int*) malloc(block_elems * sizeof(int));
  QT_block = (double*) malloc(block_elems * sizeof(double));

  for(int i = 0; i < len; i++){
    fscanf(f, "%lf\n", ts+i);
  }
  fclose(f);

  // initialize output
  for(int i=0; i<mlen; i++){
    mp[i]=-1;
    mpi[i]=-1;
  }

  for (int i = 0; i < block_elems; i++) {
      mp_block[i] = -1.0;
      mpi_block[i] = -1;
      QT_block[i] = 0.0;
    }

  // prepare statistics, initialization (some needed for the first row of matrix)
  roctxRangePush("MU");
  precom_mu(ts,mu,mlen,sublen);
  roctxRangePop();
  roctxRangePush("NORM");
  precom_norm(ts,norm,mu,mlen,sublen);
  roctxRangePop();
  roctxRangePush("DF");
  precom_df(ts,df,mlen,sublen);
  roctxRangePop();
  roctxRangePush("DG");
  precom_dg(ts,mu,dg,mlen,sublen);
  roctxRangePop();

  double tb, te;

  #pragma omp target data \
    map(to: ts[:len], mu[:mlen], df[:mlen], dg[:mlen], norm[:mlen]) \
    map(from: mp[:mlen], mpi[:mlen]) \
    map(alloc: mp_block[:block_elems], mpi_block[:block_elems], QT_block[:block_elems])
  {
    roctxRangePush("Computation");
    tb = omp_get_wtime();

    kernel(mp, mpi, ts, mu, df, dg, norm, mlen, sublen, num_blocks,
           mp_block, mpi_block, QT_block);
           
    te = omp_get_wtime();
    roctxRangePop();
  }

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
  free (ts); free(mp); free(mpi); free(mu); free(norm); free(df); free(dg);
  free (mp_block); free(mpi_block); free(QT_block);
  return 0;
}

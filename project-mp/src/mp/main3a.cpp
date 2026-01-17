#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>
#include <iostream>
#include <omp.h>

using namespace std;

template<typename T>
void print_buf(const string desc, const T *buf, const int len){
  printf("%s\n", desc.c_str());
  for (int i=0; i<len; i++)
    std::cout << buf[i] << ", ";
  printf("\n=========\n\n");
}

void precom_mu(double* ts, double* mu, int mlen, int sublen){
  double sum = 0.0;
  for (int i=0; i<sublen; i++)
    sum += ts[i];
  mu[0] = sum / sublen;

  for (int i=1; i<mlen; i++)
    mu[i] = mu[i-1] + (ts[i+sublen-1] - ts[i-1]) / sublen;
}

void precom_norm(double* ts, double* norm, double* mu, int mlen, int sublen){
  for (int i=0; i<mlen; i++){
    double sum = 0.0;
    for (int j=0; j<sublen; j++){
      double tmp = ts[i+j] - mu[i];
      sum += tmp * tmp;
    }
    norm[i] = sqrt(1.0 / sum);
  }
}

void precom_df(double* ts, double* df, int mlen, int sublen){
  df[0] = 0.0;
  for (int i=0; i<mlen-1; i++)
    df[i+1] = (ts[i+sublen] - ts[i]) * 0.5;
}

void precom_dg(double* ts, double* mu, double* dg, int mlen, int sublen){
  dg[0] = 0.0;
  for (int i=0; i<mlen-1; i++)
    dg[i+1] = ts[i+sublen] - mu[i+1] + ts[i] - mu[i];
}

/* ----- argmax reduction helper ----- */
struct Best {
  double val;
  int idx;
};

#pragma omp declare reduction( \
  bestmax : Best : \
  omp_out = (omp_in.val > omp_out.val ? omp_in : omp_out) ) \
  initializer(omp_priv = { -1.0e300, -1 })

int main(int argc, char* argv[]){
  if(argc < 4){
    printf("Usage: exe <input file> <len> <sublen>\n");
    return 0;
  }

  int len    =1<< atoi(argv[2]);
  int sublen = atoi(argv[3]);
  int mlen   = len - sublen + 1;

  FILE* f = fopen(argv[1],"r");
  if(!f){
    perror("fopen");
    return 1;
  }

  double *ts  = (double*) malloc(len  * sizeof(double));
  double *mp  = (double*) malloc(mlen * sizeof(double));
  int    *mpi = (int*)    malloc(mlen * sizeof(int));

  double *mu   = (double*) malloc(mlen * sizeof(double));
  double *norm = (double*) malloc(mlen * sizeof(double));
  double *df   = (double*) malloc(mlen * sizeof(double));
  double *dg   = (double*) malloc(mlen * sizeof(double));
  double *QT   = (double*) malloc(mlen * sizeof(double));

  for(int i=0;i<len;i++)
    fscanf(f, "%lf", ts+i);
  fclose(f);

  for(int i=0;i<mlen;i++){
    mp[i]  = -1.0;
    mpi[i] = -1;
    QT[i]  = 0.0;
  }

  precom_mu(ts,mu,mlen,sublen);
  precom_norm(ts,norm,mu,mlen,sublen);
  precom_df(ts,df,mlen,sublen);
  precom_dg(ts,mu,dg,mlen,sublen);

  /* ----- first row QT initialization ----- */
  int start = 0;
  for(int i=sublen;i<mlen;i++){
    double acc = 0.0;
    for(int j=0;j<sublen;j++)
      acc += (ts[start+j] - mu[start]) * (ts[i+j] - mu[i]);
    QT[i-sublen] = acc;
  }

  /* ============================================================
     Task 3a kernel:
     - outer i sequential (QT dependency)
     - inner j parallel
     - reduction for mp[i] (argmax)
     ============================================================ */

  Best best;

  #pragma omp parallel shared(best, mp, mpi, QT, df, dg, norm)
  {
    for(int i=0;i<mlen-sublen;i++){

      #pragma omp single
      { best.val = -1.0e300; best.idx = -1; }
      #pragma omp barrier

      #pragma omp for reduction(bestmax:best) schedule(static)
      for(int j=i+sublen;j<mlen;j++){
        int k = j - i - sublen;

        if(i!=0)
          QT[k] += df[i]*dg[j] + df[j]*dg[i];

        double cr = QT[k] * norm[i] * norm[j];

        if(cr > best.val){
          best.val = cr;
          best.idx = j;
        }

        if(cr > mp[j]){
          mp[j]  = cr;
          mpi[j] = i;
        }
      }

      #pragma omp single
      {
        if(best.idx != -1 && best.val > mp[i]){
          mp[i]  = best.val;
          mpi[i] = best.idx;
        }
      }

      #pragma omp barrier
    }
  }

  /* convert correlation to Euclidean distance */
  for(int j=0;j<mlen;j++)
    mp[j] = sqrt(2.0 * sublen * (1.0 - mp[j]));

  f = fopen("output/mp.txt","w");
  for(int i=0;i<mlen;i++)
    fprintf(f,"%f\n",mp[i]);
  fclose(f);

  f = fopen("output/mpi.txt","w");
  for(int i=0;i<mlen;i++)
    fprintf(f,"%d\n",mpi[i]);
  fclose(f);

  free(ts); free(mp); free(mpi);
  free(mu); free(norm); free(df); free(dg); free(QT);
  return 0;
}

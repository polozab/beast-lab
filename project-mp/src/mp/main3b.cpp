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
  for (int i=0; i<len; i++) std::cout << buf[i] << ", ";
  printf("\n=========\n\n");
}

void precom_mu(double* ts, double* mu, int mlen, int sublen){
  double sum = 0.0;
  for (int i=0; i<sublen; i++) sum += ts[i];
  mu[0] = sum / (double)sublen;

  // loop-carried dependency -> keep sequential
  for (int i=1; i<mlen; i++){
    mu[i] = mu[i-1] + (ts[i+sublen-1] - ts[i-1]) / (double)sublen;
  }
}

void precom_norm(double* ts, double* norm, double* mu, int mlen, int sublen){
  for (int i=0; i<mlen; i++){
    double sum = 0.0;
    for (int j=0; j<sublen; j++){
      double tmp = ts[i+j] - mu[i];
      sum += tmp*tmp;
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

static inline void update_best(double &best_val, int &best_idx, double cand_val, int cand_idx){
  if (cand_val > best_val){
    best_val = cand_val;
    best_idx = cand_idx;
  }
}

int main(int argc, char* argv[]){
  if(argc < 4){
    printf("Usage: exe <input file> <len> <sublen>\n");
    return 0;
  }

  int len    =1<< atoi(argv[2]);
  int sublen = atoi(argv[3]);
  int mlen   = len - sublen + 1;

  FILE* f = fopen(argv[1], "r");
  if(!f){
    perror("fopen");
    return 1;
  }

  double *ts   = (double*) malloc((size_t)len  * sizeof(double));
  double *mp   = (double*) malloc((size_t)mlen * sizeof(double));
  int    *mpi  = (int*)    malloc((size_t)mlen * sizeof(int));
  double *mu   = (double*) malloc((size_t)mlen * sizeof(double));
  double *norm = (double*) malloc((size_t)mlen * sizeof(double));
  double *df   = (double*) malloc((size_t)mlen * sizeof(double));
  double *dg   = (double*) malloc((size_t)mlen * sizeof(double));

  for(int i=0;i<len;i++){
    if (fscanf(f, "%lf", &ts[i]) != 1){
      printf("Error: input has fewer than LEN=%d values.\n", len);
      fclose(f);
      return 1;
    }
  }
  fclose(f);

  for(int i=0;i<mlen;i++){
    mp[i]  = -1.0;
    mpi[i] = -1;
  }

  // precompute (same as baseline)
  precom_mu(ts, mu, mlen, sublen);
  precom_norm(ts, norm, mu, mlen, sublen);
  precom_df(ts, df, mlen, sublen);
  precom_dg(ts, mu, dg, mlen, sublen);

  // ---------------------------
  // Task 3b: Privatize outputs
  // ---------------------------
  const int T = omp_get_max_threads();
  const double NEG_INF = -1.0e300;

  double *mp_loc  = (double*) malloc((size_t)T * (size_t)mlen * sizeof(double));
  int    *mpi_loc = (int*)    malloc((size_t)T * (size_t)mlen * sizeof(int));

  // init thread-local outputs
  #pragma omp parallel
  {
    int tid = omp_get_thread_num();
    double *mp_t  = mp_loc  + (size_t)tid * (size_t)mlen;
    int    *mpi_t = mpi_loc + (size_t)tid * (size_t)mlen;

    #pragma omp for schedule(static)
    for(int k=0;k<mlen;k++){
      mp_t[k]  = NEG_INF;
      mpi_t[k] = -1;
    }
  }

  // ------------------------------------------
  // Task 3b: Parallelize outer i with private QT
  // - each thread gets a contiguous i-range
  // - each thread initializes QT for its istart
  // - then streams QT inside its own range
  // - updates go only to thread-local mp_t/mpi_t
  // ------------------------------------------
  const int Imax = mlen - sublen; // outer loop upper bound

  #pragma omp parallel
  {
    int tid = omp_get_thread_num();
    int nt  = omp_get_num_threads();

    int istart = (int)((long long)Imax * tid / nt);
    int iend   = (int)((long long)Imax * (tid + 1) / nt);

    double *mp_t  = mp_loc  + (size_t)tid * (size_t)mlen;
    int    *mpi_t = mpi_loc + (size_t)tid * (size_t)mlen;

    // private QT buffer (max needed for this thread's istart)
    int qt_len_max = mlen - istart - sublen;
    if (qt_len_max < 0) qt_len_max = 0;
    double *QT = (double*) malloc((size_t)qt_len_max * sizeof(double));

    // initialize QT for i = istart:
    // QT[k] = dot( (ts[istart..istart+sublen-1]-mu[istart]),
    //              (ts[j..j+sublen-1]-mu[j]) )
    // where j = istart + sublen + k
    if (istart < iend && qt_len_max > 0){
      const double mui = mu[istart];

      for(int k=0;k<qt_len_max;k++){
        int j = istart + sublen + k;
        const double muj = mu[j];
        double acc = 0.0;
        for(int t=0;t<sublen;t++){
          acc += (ts[istart + t] - mui) * (ts[j + t] - muj);
        }
        QT[k] = acc;
      }
    }

    // stream over this thread's i-range
    for(int i=istart; i<iend; i++){
      int qt_len = mlen - i - sublen; // valid k range for this i
      if (qt_len <= 0) break;

      for(int j=i+sublen; j<mlen; j++){
        int k = j - i - sublen;

        if (i != istart){
          // streaming update: QT[k] corresponds to diagonal (i-1, j-1) -> (i, j)
          QT[k] += df[i] * dg[j] + df[j] * dg[i];
        }

        double cr = QT[k] * norm[i] * norm[j];

        // update local best for i and j (no races: thread-local arrays)
        update_best(mp_t[i], mpi_t[i], cr, j);
        update_best(mp_t[j], mpi_t[j], cr, i);
      }
    }

    free(QT);
  }

  // ---------------------------
  // Merge thread-local results
  // ---------------------------
  #pragma omp parallel for schedule(static)
  for(int k=0;k<mlen;k++){
    double best_val = NEG_INF;
    int best_idx = -1;
    for(int t=0;t<T;t++){
      double v = mp_loc[(size_t)t*(size_t)mlen + (size_t)k];
      if (v > best_val){
        best_val = v;
        best_idx = mpi_loc[(size_t)t*(size_t)mlen + (size_t)k];
      }
    }
    mp[k]  = best_val;
    mpi[k] = best_idx;
  }

  free(mp_loc);
  free(mpi_loc);

  // convert correlation -> Euclidean distance
  #pragma omp parallel for schedule(static)
  for(int k=0;k<mlen;k++){
    mp[k] = sqrt(2.0 * (double)sublen * (1.0 - mp[k]));
  }

  // write results
  f = fopen("output/mp.txt","w");
  for(int i=0;i<mlen;i++) fprintf(f, "%f\n", mp[i]);
  fclose(f);

  f = fopen("output/mpi.txt","w");
  for(int i=0;i<mlen;i++) fprintf(f, "%d\n", mpi[i]);
  fclose(f);

  free(ts); free(mp); free(mpi); free(mu); free(norm); free(df); free(dg);
  return 0;
}

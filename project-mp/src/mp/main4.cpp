#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <omp.h>

static inline double neg_inf() { return -1.0e300; }

void precom_mu(const double* ts, double* mu, int mlen, int sublen){
  double sum = 0.0;
  for(int i=0;i<sublen;i++) sum += ts[i];
  mu[0] = sum / (double)sublen;

  // dependency -> keep sequential
  for(int i=1;i<mlen;i++)
    mu[i] = mu[i-1] + (ts[i+sublen-1] - ts[i-1]) / (double)sublen;
}

void precom_norm(const double* ts, const double* mu, double* norm, int mlen, int sublen){
  // keep sequential (Task 3c style). If you want SIMD later, we can add it safely.
  for(int i=0;i<mlen;i++){
    double sum = 0.0;
    for(int j=0;j<sublen;j++){
      double tmp = ts[i+j] - mu[i];
      sum += tmp*tmp;
    }
    norm[i] = sqrt(1.0 / sum);
  }
}

int main(int argc, char* argv[]){
  if(argc < 4){
    printf("Usage: exe <input file> <len_log2> <sublen> [device_id]\n");
    return 0;
  }

  const char* path = argv[1];
  int len_log2 = atoi(argv[2]);
  int len      = 1 << len_log2;          // requested
  int sublen   = atoi(argv[3]);
  int dev_id   = (argc >= 5) ? atoi(argv[4]) : 0;

  int mlen = len - sublen + 1;
  if(mlen <= 0){
    printf("Error: len must be >= sublen\n");
    return 1;
  }

  omp_set_default_device(dev_id);

  FILE* f = fopen(path, "r");
  if(!f){
    perror("fopen");
    return 1;
  }

  double* ts   = (double*)malloc((size_t)len  * sizeof(double));
  double* mu   = (double*)malloc((size_t)mlen * sizeof(double));
  double* norm = (double*)malloc((size_t)mlen * sizeof(double));
  double* mp   = (double*)malloc((size_t)mlen * sizeof(double));
  int*    mpi  = (int*)   malloc((size_t)mlen * sizeof(int));

  for(int i=0;i<len;i++){
    if(fscanf(f, "%lf", &ts[i]) != 1){
      printf("Error: input shorter than len=%d\n", len);
      fclose(f);
      return 1;
    }
  }
  fclose(f);

  // init
  for(int i=0;i<mlen;i++){
    mp[i]  = neg_inf();
    mpi[i] = -1;
  }

  // precompute on host (simple + stable). We can offload later.
  precom_mu(ts, mu, mlen, sublen);
  precom_norm(ts, mu, norm, mlen, sublen);

  // Optional: print device info once (host-side)
  int num_dev = omp_get_num_devices();
  printf("len=%d sublen=%d mlen=%d device=%d num_devices=%d\n",
         len, sublen, mlen, dev_id, num_dev);

  // -------------------------------
  // GPU OFFLOAD KERNEL (baseline)
  //
  // Compute mp[i], mpi[i] independently:
  //   mp[i] = max_j corr(i,j) with exclusion zone |i-j| >= sublen
  //
  // No shared updates -> no atomics/critical/reduction.
  // -------------------------------
  #pragma omp target teams distribute parallel for \
    map(to: ts[0:len], mu[0:mlen], norm[0:mlen]) \
    map(tofrom: mp[0:mlen], mpi[0:mlen])
  for(int i=0;i<mlen;i++){
    double best = neg_inf();
    int bestj = -1;

    // exclude trivial matches: |i-j| < sublen
    for(int j=0;j<mlen;j++){
      int dist = i - j; if(dist < 0) dist = -dist;
      if(dist < sublen) continue;
      // dot product of z-normalized subsequences:
      // QT = sum_t (ts[i+t]-mu[i])*(ts[j+t]-mu[j])
      double acc = 0.0;
      double mui = mu[i];
      double muj = mu[j];

      // NOTE: this loop is a perfect SIMD candidate later (on device too),
      // but we keep it simple/correct first.
      for(int t=0;t<sublen;t++){
        acc += (ts[i+t] - mui) * (ts[j+t] - muj);
      }

      double cr = acc * norm[i] * norm[j];
      if(cr > best){
        best = cr;
        bestj = j;
      }
    }

    mp[i]  = best;
    mpi[i] = bestj;
  }

  // Convert correlation -> Euclidean distance (host)
  for(int i=0;i<mlen;i++){
    mp[i] = sqrt(2.0 * (double)sublen * (1.0 - mp[i]));
  }

  // write results
  FILE* fo = fopen("output/mp.txt", "w");
  if(!fo){ perror("fopen mp.txt"); return 1; }
  for(int i=0;i<mlen;i++) fprintf(fo, "%f\n", mp[i]);
  fclose(fo);

  fo = fopen("output/mpi.txt", "w");
  if(!fo){ perror("fopen mpi.txt"); return 1; }
  for(int i=0;i<mlen;i++) fprintf(fo, "%d\n", mpi[i]);
  fclose(fo);

  free(ts); free(mu); free(norm); free(mp); free(mpi);
  return 0;
}

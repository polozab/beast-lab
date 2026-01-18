#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <omp.h>

static inline double neg_inf() { return -1.0e300; }

void precom_mu(const double* ts, double* mu, int mlen, int sublen){
  double sum = 0.0;
  for(int i=0;i<sublen;i++) sum += ts[i];
  mu[0] = sum / (double)sublen;

  // dependency -> sequential
  for(int i=1;i<mlen;i++)
    mu[i] = mu[i-1] + (ts[i+sublen-1] - ts[i-1]) / (double)sublen;
}

void precom_norm(const double* ts, const double* mu, double* norm, int mlen, int sublen){
  for(int i=0;i<mlen;i++){
    double sum = 0.0;
    for(int j=0;j<sublen;j++){
      double tmp = ts[i+j] - mu[i];
      sum += tmp*tmp;
    }
    norm[i] = sqrt(1.0 / sum);
  }
}

static inline void usage(){
  printf("Usage: exe <input> <len_log2> <sublen> [start_device] [num_devices] [num_teams] [threads_per_team]\n");
}

int main(int argc, char* argv[]){
  if(argc < 4){
    usage();
    return 0;
  }

  const char* path = argv[1];
  int len_log2 = atoi(argv[2]);
  int len      = 1 << len_log2;
  int sublen   = atoi(argv[3]);
  int mlen     = len - sublen + 1;

  int start_dev = (argc >= 5) ? atoi(argv[4]) : 0;
  int req_ndev  = (argc >= 6) ? atoi(argv[5]) : 0; // 0 -> all available from start_dev
  int NUM_TEAMS = (argc >= 7) ? atoi(argv[6]) : 0; // 0 -> runtime chooses
  int THREADS   = (argc >= 8) ? atoi(argv[7]) : 0; // 0 -> runtime chooses

  if(mlen <= 0){
    printf("Error: len must be >= sublen\n");
    return 1;
  }

  int ndev_total = omp_get_num_devices();
  if(ndev_total <= 0){
    printf("Error: omp_get_num_devices() returned %d (no target devices)\n", ndev_total);
    return 1;
  }
  if(start_dev < 0) start_dev = 0;
  if(start_dev >= ndev_total){
    printf("Error: start_device=%d but only %d devices available\n", start_dev, ndev_total);
    return 1;
  }

  int ndev_use = (req_ndev <= 0) ? (ndev_total - start_dev) : req_ndev;
  if(start_dev + ndev_use > ndev_total) ndev_use = ndev_total - start_dev;
  if(ndev_use <= 0){
    printf("Error: no devices selected (start=%d, req=%d, total=%d)\n", start_dev, req_ndev, ndev_total);
    return 1;
  }

  printf("len=%d (2^%d) sublen=%d mlen=%d | devices_total=%d | using devices [%d..%d]\n",
         len, len_log2, sublen, mlen, ndev_total, start_dev, start_dev + ndev_use - 1);
  printf("requested launch: num_teams=%d threads_per_team=%d\n", NUM_TEAMS, THREADS);

  // ---------- Read input ----------
  FILE* f = fopen(path, "r");
  if(!f){ perror("fopen"); return 1; }

  double* ts   = (double*)malloc((size_t)len  * sizeof(double));
  double* mu   = (double*)malloc((size_t)mlen * sizeof(double));
  double* norm = (double*)malloc((size_t)mlen * sizeof(double));
  double* mp   = (double*)malloc((size_t)mlen * sizeof(double));
  int*    mpi  = (int*)   malloc((size_t)mlen * sizeof(int));

  for(int i=0;i<len;i++){
    if(fscanf(f, "%lf", &ts[i]) != 1){
      printf("Error: input file shorter than len=%d\n", len);
      fclose(f);
      return 1;
    }
  }
  fclose(f);

  for(int i=0;i<mlen;i++){
    mp[i]  = neg_inf();
    mpi[i] = -1;
  }

  // Precompute on host (stable baseline). We can offload later if wanted.
  precom_mu(ts, mu, mlen, sublen);
  precom_norm(ts, mu, norm, mlen, sublen);

  // ---------- Multi-GPU offload ----------
  // Split i in contiguous chunks across devices.
  // Each device writes only its slice mp[i0:i1), mpi[i0:i1) -> no merge needed.
  #pragma omp parallel num_threads(ndev_use)
  {
    int local = omp_get_thread_num();
    int dev   = start_dev + local;

    // chunk for this device
    int i0 = (int)((long long)mlen * local / ndev_use);
    int i1 = (int)((long long)mlen * (local + 1) / ndev_use);
    int count = i1 - i0;

    omp_set_default_device(dev);

    // (Optional) device-side debug (prints multiple times across devices; OK)
    // We print once per device by guarding on i == i0.
    #pragma omp target teams distribute parallel for \
        map(to: ts[0:len], mu[0:mlen], norm[0:mlen]) \
        map(tofrom: mp[i0:count], mpi[i0:count]) \
        num_teams(NUM_TEAMS) thread_limit(THREADS)
    for(int i=i0; i<i1; i++){
      if(i == i0){
        printf("[DEVICE %d] actual: teams=%d threads=%d | slice=[%d,%d)\n",
               dev, omp_get_num_teams(), omp_get_num_threads(), i0, i1);
      }

      double best = neg_inf();
      int bestj = -1;

      // independent per i -> safe
      for(int j=0;j<mlen;j++){
        int d = i - j; if(d < 0) d = -d;
        if(d < sublen) continue; // exclusion zone

        double acc = 0.0;
        double mui = mu[i];
        double muj = mu[j];

        // dot product over sublen
        for(int t=0;t<sublen;t++){
          acc += (ts[i+t] - mui) * (ts[j+t] - muj);
        }

        double cr = acc * norm[i] * norm[j];

        // tie-break: pick smaller j if equal (more deterministic)
        if(cr > best || (cr == best && (bestj < 0 || j < bestj))){
          best = cr;
          bestj = j;
        }
      }

      mp[i]  = best;
      mpi[i] = bestj;
    }
  }

  // Convert correlation -> Euclidean distance on host
  for(int i=0;i<mlen;i++){
    mp[i] = sqrt(2.0 * (double)sublen * (1.0 - mp[i]));
  }

  // Write results
  FILE* fo = fopen("output/mp.txt","w");
  if(!fo){ perror("fopen mp.txt"); return 1; }
  for(int i=0;i<mlen;i++) fprintf(fo, "%f\n", mp[i]);
  fclose(fo);

  fo = fopen("output/mpi.txt","w");
  if(!fo){ perror("fopen mpi.txt"); return 1; }
  for(int i=0;i<mlen;i++) fprintf(fo, "%d\n", mpi[i]);
  fclose(fo);

  free(ts); free(mu); free(norm); free(mp); free(mpi);
  return 0;
}

## Approach 1a - Parallelization over diagonals

### Solving QT access

This approach is based on the observation that each index of the `QT` is accessed in a specific pattern. This would potentially allow us to reorder the loops in such a way that individual indexes are owned by only one team or thread. We only head to update the outer loop and generate a private copy of the QT element in which we accumulate to when iterating over the inner loop. The outer loop represents the individual diagonals of the matrix while the inner loop represents the single elements of that diagonal:

```cpp
#pragma omp target teams distribute num_teams(num_teams)
for (int k = 0; k < mlen-sublen; k++) {

  double QTk = QT[k];

  for (int i = 0; i < mlen-sublen-k; i++) {

    int j = i+sublen+k;

    if (i != 0) {
      QTk += df[i]*dg[j] + df[j]*dg[i];
    }

    double cr = QTk * norm[i] * norm[j];
    ...
  }

  QT[k] = QTk;
}
```

This approach would eliminate the first dependency as now every outer loop iteration only ever accesses one diagonal. However the other data race still exists and to solve it we had to somehow make sure the comparisons and assignments are done in an atomic fashion. 

### Solving the mp and mpi accesses

Solving this problem was even more complicated as we have two separate accesses with completely different indexes. This means it is not possible to make one thread own an unique index that accesses only one element of `mp` and `mpi`, as in every iteration index `i` and `j` get accessed. 

The first option would be to use atomic operations provided by OpenMP. However this is not easily done as even more complicated we also update two separate arrays within a max-check so we are not able to use OpenMP directives like `capture` or `update`. Instead we would have to fallback to the general solution by using OpenMPs `critical` structure. After testing this approach we quickly realized that synchronization would be very difficult as it massively affected the performance. This would also rule out any other approaches that try to at least git rid of a single atomic comparison by parallelizing over `i` instead, as even a single critical section (ignoring that this leads to invalid results) was enough to tank the performance completely.

```cpp
#pragma omp target teams distribute num_teams(num_teams)
for (int k = 0; k < mlen-sublen; k++) {

  double QTk = QT[k];

  for (int i = 0; i < mlen-sublen-k; i++) {
    ...

    #pragma omp critical {
      if (cr > local_mp[i]) {
        local_mp[i] = cr;
        local_mpi[i] = j;
      }
    }

    #pragma omp critical {
      if (cr > local_mp[j]) {
        local_mp[j] = cr;
        local_mpi[j] = i;
      }
    }
  }

  QT[k] = QTk;
}
```

The second option we opted for was to initialize a per team buffer that keeps track of the team local best `mp` and `mpi` values. These could at the end be reduced to the real maximum value. One huge disadvantage of this approach is that our memory footprint is very high as the entire matrix length is kept in memory for the number of teams. 

We decided to initialize the team-local `mp` `mpi` on the GPU itself by using device pointers with `omp_target_alloc` which was done by using the following setup:

```cpp
const int dev  = omp_get_default_device();
const int team_mlen = num_teams * mlen;

double *team_mp  = (double*) omp_target_alloc(team_mlen * sizeof(double), dev);
int    *team_mpi = (int*)    omp_target_alloc(team_mlen * sizeof(int), dev);

#pragma omp target teams distribute parallel for is_device_ptr(team_mp, team_mpi)
for (int idx = 0; idx < team_mlen; ++idx) {
  team_mp[idx]  = -1;
  team_mpi[idx] = -1;
}
```

To be able to access the other data necessary for the computational kernel we reused the `map` statement of the OpenMP `target data` structure. Only the offloaded region itself had to be changed to instead use `team_mp` `team_mpi` as device pointers. Additionally the kernel was rewritten to first calculate the offset where the teams private buffer in the array sits and the accesses at the end of the inner loop now point to these private buffers:

```cpp
#pragma omp target teams num_teams(num_teams) is_device_ptr(team_mp, team_mpi)
{
  const int team = omp_get_team_num();

  double *local_mp = team_mp + (size_t)team * mlen;
  int    *local_mpi = team_mpi + (size_t)team * mlen;

  #pragma omp distribute
  for (int k = 0; k < mlen - sublen; k++) {
    for (int i = 0; i < mlen - sublen - k; i++) {
      int j = i + sublen + k;
      ...

      if (cr > local_mp[i]) { local_mp[i] = cr; local_mpi[i] = j; }
      if (cr > local_mp[j]) { local_mp[j] = cr; local_mpi[j] = i; }
    }
  }
}
```

To complete this approach we also had to do a reduction of the individual team private result buffers. This was easily done by creating a appropriate reduction kernel which was sufficiently parallelized:

```cpp
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
```

With that we had our first working race-free implementation of the matrix profile kernel running on the GPU.

### Performance evaluation

After examining the performance of this kernel we quickly realized some critical bottlenecks must limit this kernel, which cause the abysmal performance results of only `6,8 GLOPS/s` when executing with a matrix size of `2^16` and window size of `10`.

First of all we suspect that the parallelism by only distributing over teams is not sufficient to fully utilize the entire GPU. However this cannot easily be fixed as the amount of parallel work we can do is directly proportional to the amount of memory we consume, due to the use of team private buffers. This is supported by the fact that we increase our performance with an increase in number of teams until a certain threshold. Our current optimal values is `4048` team. After that the performance goes down again. One positive note is that reduction kernel has a negatable impact on the overall performance even for such high team numbers.

Another limiting factor we would suspected for the computational kernel on a theoretical level is that the work distribution to the teams is not uniform as every diagonal has a different length. This internally means that the team scheduler has the difficult task to manage all the distribution efficiently, which is definitely possible, however unlikely to happen without further hints or a more predictable loop pattern. This we did not confirm in reality however is likely one issue causing this barely faster performance by the GPU.

For these reasons we concluded this approach would not be feasible to optimize efficiently with our limited time. However we did have some thoughts on improving this approach that we could explore in the future.

## Approach 1b - Two level parallelism & Tiling

### Two-level parallelism

The first idea we would like to try is to just do a double-reduction and distribute over teams and threads. We would have to reduce the number of team overall but this may help to use the hierarchy of the provided GPU architecture more effectively. As we concluded that doing the reduction operation has no significant impact doing it over both threads and teams would not severely affect the performance.

To implement this we first of all had to allocate memory according to every possible thread of every team. We say possible, as we only use the OpenMP `max_threads` directive to determine an upper limit. If the runtime chooses to lower the number of threads due to some reason it was possible. We also tried specifying a fixed number of threads but this made no significant difference in performance. The allocation was as follows:

```cpp
const int dev  = omp_get_default_device();
const int block_mlen = num_threads * num_teams * mlen;
double *block_mp = (double*) omp_target_alloc(block_mlen * sizeof(double), dev);
int *block_mpi = (int*) omp_target_alloc(block_mlen * sizeof(int), dev);
```

We also had to adapt the reduction kernel to go over every thread in every team instead, which was a quick thing to implement:

```cpp
#pragma omp target teams distribute parallel for is_device_ptr(block_mp, block_mpi)
for (int i = 0; i < mlen; i++) {
  ...

  for (int team = 0; team < num_teams; team++) {
    for (int thread = 0; thread < num_threads; thread++) {
      int offset = (team * num_threads + thread) * mlen + i;
      ...
    }
  }
}
```

### Tiling of matrix into blocks

However our current loop setup doesn't fully utilize the hierarchy of the GPU yet. To be able to do this we opted to implement tiling for both the range of visited `k` and `i`, which get assigned to different teams. Thus we would get blocks of the matrix distributed to different computing units reducing the overall problem size that they have to work on. The implementation utilizes the OpenMP `collapse` directive to distribute both block-tile loops:

```cpp
...
#pragma omp distribute parallel for collapse(2)
for (int kt = 0; kt < rows; kt += tiles_k) {
  for (int it = 0; it < rows; it += tiles_i) {
    ...
  }
}
```

The inner block is semantically mostly identical to our previous kernel with the addition of using the block-tile indexes instead. To circumvent overflows we also made sure to check if the index of a tile overflows the actual matrix dimensions by using `min` statements:

```cpp
const int thread = omp_get_thread_num();
double *local_mp = block_mp + (team*num_threads + thread)*mlen;
int *local_mpi = block_mpi + (team*num_threads + thread)*mlen;

for (int k = kt; k < min(kt+tiles_k, rows); k++) {

  int i0 = it;
  int i1 = min(it + tiles_i, rows - k);
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
```

The inside of the `k`-tiles was iterated over by the outer loop which visited every the individual diagonals. For every diagonal we also calculated the correct starting `QT` using the same method provided by the assignment however only for the specific diagonal instead of the whole first line. This allowed us to completely get rid of the allocation of the `QT` buffer on both the host and device. This improved our usage of memory slightly as we only allocate so many private `QT` elements as we create team which are are less compared to the matrix size. The update of the result buffers were not affected by this change only the pointers now point at the thread-private buffers instead of the team-private ones.

### Redistributing parallelism



In addition we further instantiate a number of threads to handle different blocks of diagonals within the block of the team. 


## Future improvements

In addition to doing parallelism on two-level we could also distribute different tiles of the matrix to separate devices. This would not only be yet another level of parallelism but would also increase the amount of available GPU memory. With that two-level parallelism with thread-private result buffers could become feasibly even at huge matrix sizes like `2^20`. We think that this improvement would be the next step trying to get this approach to a reasonable performance and we would be very interested to try this out if we have the time.

Another possible improvement to our current kernel would be to putting even more effort and time into tuning the parameters of the kernel. Unfortunately we didn't quite finish our complete testing until the deadline and if we had additional time it might be possible to archive an even better performance. As we saw these have a significant impact on the kernel and would be worth to investigate further.

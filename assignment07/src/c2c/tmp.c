#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>
#include <stdatomic.h>

// Control block to orchestrate a single persistent team:
// - epoch increments for each bench
// - done marks completion for that epoch
// - arrived counts when both participants finished
// - t1/t2 identify participating thread ids
// - addr points to the cache-line used this epoch
typedef struct {
    _Atomic int epoch;
    _Atomic int done;
    _Atomic int arrived;
    _Atomic int t1;
    _Atomic int t2;
    _Atomic intptr_t addr;
} ctrl;

static inline void pingpong_worker(_Atomic int *addr, int tid, int t1, int t2, int pings)
{
    if (tid == t1) {
        atomic_store_explicit(addr, 0, memory_order_release);
        for (int c = 0; c < pings; c++) {
            atomic_store_explicit(addr, 1, memory_order_release);
            while (atomic_load_explicit(addr, memory_order_acquire) == 1);
        }
        atomic_store_explicit(addr, 2, memory_order_release);
    } else if (tid == t2) {
        while (atomic_load_explicit(addr, memory_order_acquire) != 2) {
            while (atomic_load_explicit(addr, memory_order_acquire) == 0);
            if (atomic_load_explicit(addr, memory_order_acquire) == 1)
                atomic_store_explicit(addr, 0, memory_order_release);
        }
    }
}

int main(void)
{
    int tc = omp_get_max_threads();
    int pings = 1000;
    int *buf = malloc(64);
    if (!buf) return 1;

    ctrl c = {0};
    atomic_store(&c.epoch, 0);
    atomic_store(&c.done, 0);
    atomic_store(&c.arrived, 0);
    atomic_store(&c.t1, -1);
    atomic_store(&c.t2, -1);
    atomic_store(&c.addr, (intptr_t)buf);

    #pragma omp parallel num_threads(tc) shared(c)
    {
        int tid = omp_get_thread_num();
        int last_epoch = 0;

        if (tid == 0) {
            int master_epoch = 0;
            // Example: walk all pairs once
            for (int t1 = 0; t1 < tc; t1++) {
                for (int t2 = 0; t2 < tc; t2++) {
                    if (t1 == t2) continue;
                    atomic_store_explicit(&c.arrived, 0, memory_order_relaxed);
                    atomic_store_explicit(&c.done, 0, memory_order_relaxed);
                    atomic_store_explicit(&c.t1, t1, memory_order_relaxed);
                    atomic_store_explicit(&c.t2, t2, memory_order_relaxed);
                    atomic_store_explicit(&c.addr, (intptr_t)buf, memory_order_relaxed);
                    atomic_store_explicit(&c.epoch, ++master_epoch, memory_order_release);
                    while (atomic_load_explicit(&c.done, memory_order_acquire) != master_epoch);
                }
            }
            atomic_store_explicit(&c.epoch, -1, memory_order_release); // stop workers
        } else {
            for (;;) {
                int e = atomic_load_explicit(&c.epoch, memory_order_acquire);
                if (e == last_epoch) continue;
                last_epoch = e;
                if (e < 0) break;

                int t1 = atomic_load_explicit(&c.t1, memory_order_acquire);
                int t2 = atomic_load_explicit(&c.t2, memory_order_acquire);
                if (tid == t1 || tid == t2) {
                    _Atomic int *addr_ptr = (_Atomic int *)atomic_load_explicit(&c.addr, memory_order_acquire);
                    pingpong_worker(addr_ptr, tid, t1, t2, pings);
                    int arrived = atomic_fetch_add_explicit(&c.arrived, 1, memory_order_acq_rel) + 1;
                    if (arrived == 2)
                        atomic_store_explicit(&c.done, e, memory_order_release);
                }
            }
        }
    }

    free(buf);
    return 0;
}

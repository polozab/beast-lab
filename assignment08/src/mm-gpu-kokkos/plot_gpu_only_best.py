import matplotlib.pyplot as plt

# (N, best_time_s, best_gflops) from GPU-only best-of-6 runs
DATA = [
    (256, 0.0153042, 2.1925),
    (512, 0.0153573, 17.4794),
    (1024, 0.020704, 103.723),
    (2048, 0.0751878, 228.493),
    (3072, 0.226164, 256.372),
    (4096, 0.521727, 263.431),
    (5120, 1.02108, 262.895),
    (6144, 1.75421, 264.424),
    (7168, 2.79211, 263.81),
    (8192, 3.96987, 276.964),
    (9216, 5.88655, 265.948),
    (10000, 7.65688, 261.203),
]


def bytes_moved(n):
    # Simple model: read A, read B, write C (FP64)
    return 3 * n * n * 8


def main():
    bw_rows = []
    for n, t, g in DATA:
        gb = bytes_moved(n) / 1e9
        bw = gb / t
        bw_rows.append((n, t, g, bw))

    # Plot GFLOP/s vs N
    fig, ax = plt.subplots(figsize=(8, 4))
    ax.plot([r[0] for r in DATA], [r[2] for r in DATA], marker="o")
    ax.set_xlabel("N")
    ax.set_ylabel("GFLOP/s")
    ax.set_title("kokkos_mm_GPU_Only (best of runs)")
    ax.grid(True, linestyle="--", alpha=0.3)
    plt.tight_layout()
    plt.savefig("gpu_only_best.png", dpi=150)
    print("Saved gpu_only_best.png")

    print("N    time_s    GFLOP/s    GB/s (3*N^2*8 bytes)")
    for n, t, g, bw in bw_rows:
        print(f"{n:<5d} {t:<8.5f} {g:<8.3f} {bw:<8.3f}")


if __name__ == "__main__":
    main()

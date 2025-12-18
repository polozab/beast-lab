import matplotlib.pyplot as plt


def load_rows(path):
    rows = []
    with open(path) as f:
        for line in f:
            if line.startswith("Row-wise") or line.startswith("N ") or not line.strip():
                continue
            parts = line.split()
            if len(parts) != 6:
                continue
            N, teams, threads, vec, time_s, gflops = parts
            rows.append(
                {
                    "N": int(N),
                    "teams": int(teams),
                    "threads": int(threads),
                    "vec": int(vec),
                    "gflops": float(gflops),
                }
            )
    return rows


def main():
    rows = load_rows("rowwise_results.txt")
    vec_vals = sorted({r["vec"] for r in rows})
    fig, axes = plt.subplots(1, len(vec_vals), figsize=(12, 4), sharey=True)
    if len(vec_vals) == 1:
        axes = [axes]
    colors = {64: "tab:blue", 128: "tab:orange", 256: "tab:green"}

    for ax, vec in zip(axes, vec_vals):
        sub = [r for r in rows if r["vec"] == vec]
        ax.set_title(f"vec_len={vec}")
        for threads in sorted({r["threads"] for r in sub}):
            pts = sorted([r for r in sub if r["threads"] == threads], key=lambda x: x["N"])
            ax.plot(
                [p["N"] for p in pts],
                [p["gflops"] for p in pts],
                marker="o",
                color=colors.get(threads, "gray"),
                label=f"threads={threads}",
            )
        ax.set_xlabel("N")
        ax.grid(True, linestyle="--", alpha=0.3)
    axes[0].set_ylabel("GFLOP/s")
    axes[-1].legend(title="threads", loc="best")
    plt.tight_layout()
    plt.savefig("rowwise_perf.png", dpi=150)
    print("Saved plot to rowwise_perf.png")


if __name__ == "__main__":
    main()

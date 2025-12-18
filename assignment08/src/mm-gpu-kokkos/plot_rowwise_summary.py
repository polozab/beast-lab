import matplotlib.pyplot as plt


def load_rows(path: str):
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
                    "time_s": float(time_s),
                    "gflops": float(gflops),
                    "teams_norm": int(teams) / int(N),
                }
            )
    return rows


def main():
    data = load_rows("rowwise_results.txt")
    vec_vals = sorted({r["vec"] for r in data})
    thread_vals = sorted({r["threads"] for r in data})
    colors = {64: "tab:blue", 128: "tab:orange", 256: "tab:green"}
    markers = {1: "o", 2: "s", 4: "^"}

    n_panels = 2 + len(vec_vals)
    fig, axes = plt.subplots(1, n_panels, figsize=(4 * n_panels, 4), sharey=True)

    # Panel 1: GFLOP/s vs N, one line per (vec,threads)
    ax = axes[0]
    ax.set_title("GFLOP/s vs N")
    for vec in vec_vals:
        for threads in thread_vals:
            sub = sorted(
                [r for r in data if r["vec"] == vec and r["threads"] == threads],
                key=lambda x: x["N"],
            )
            if not sub:
                continue
            ax.plot(
                [r["N"] for r in sub],
                [r["gflops"] for r in sub],
                marker=markers.get(vec, "o"),
                color=colors.get(threads, "gray"),
                label=f"vec={vec}, threads={threads}",
                alpha=0.8,
            )
    ax.set_xlabel("N")
    ax.set_ylabel("GFLOP/s")
    ax.grid(True, linestyle="--", alpha=0.3)
    ax.legend(fontsize=7)

    # Panel 2: GFLOP/s vs teams/N scatter (overall view)
    ax = axes[1]
    ax.set_title("GFLOP/s vs teams/N")
    for r in data:
        ax.scatter(
            r["teams_norm"],
            r["gflops"],
            color=colors.get(r["threads"], "gray"),
            marker=markers.get(r["vec"], "o"),
            alpha=0.6,
        )
    ax.set_xlabel("teams / N")
    ax.grid(True, linestyle="--", alpha=0.3)

    # Remaining panels: Small multiples by vec_len (GFLOP/s vs teams_norm)
    for idx, vec in enumerate(vec_vals, start=2):
        ax = axes[idx]
        ax.set_title(f"vec_len={vec}")
        sub = [r for r in data if r["vec"] == vec]
        for threads in thread_vals:
            pts = sorted(
                [r for r in sub if r["threads"] == threads],
                key=lambda x: x["teams_norm"],
            )
            if not pts:
                continue
            ax.plot(
                [p["teams_norm"] for p in pts],
                [p["gflops"] for p in pts],
                marker="o",
                color=colors.get(threads, "gray"),
                label=f"threads={threads}",
                alpha=0.8,
            )
        ax.set_xlabel("teams / N")
        ax.grid(True, linestyle="--", alpha=0.3)
        ax.legend(fontsize=7)

    plt.tight_layout()
    plt.savefig("rowwise_summary.png", dpi=150)
    print("Saved rowwise_summary.png")


if __name__ == "__main__":
    main()

import pandas as pd
import matplotlib.pyplot as plt


def main():
    df = pd.read_csv(
        "rowwise_results.txt",
        delim_whitespace=True,
        skiprows=2,
        names=["N", "teams", "threads", "vec", "time_s", "gflops"],
    )
    df["teams_norm"] = df["teams"] / df["N"]

    fig, ax = plt.subplots(figsize=(8, 5))
    for vec, marker in zip(sorted(df["vec"].unique()), ["o", "s", "^", "v"]):
        sub = df[df["vec"] == vec]
        sc = ax.scatter(
            sub["teams_norm"],
            sub["gflops"],
            c=sub["threads"],
            cmap="viridis",
            marker=marker,
            alpha=0.8,
            label=f"vec={vec}",
        )

    ax.set_xlabel("teams / N")
    ax.set_ylabel("GFLOP/s")
    ax.set_title("Row-wise Teams sweep")
    cbar = fig.colorbar(sc, ax=ax, label="threads")
    ax.legend(title="vec_len")
    ax.grid(True, linestyle="--", alpha=0.3)
    plt.tight_layout()
    plt.savefig("rowwise_perf.png", dpi=150)
    print("Saved plot to rowwise_perf.png")


if __name__ == "__main__":
    main()

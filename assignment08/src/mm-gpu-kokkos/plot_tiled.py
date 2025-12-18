import pandas as pd
import matplotlib.pyplot as plt


def main():
    df = pd.read_csv(
        "tiled_results_6144.txt",
        delim_whitespace=True,
        comment="L",
        skiprows=2,
        names=["tile_m", "tile_n", "team", "vec", "time_s", "gflops"],
    )

    fig, ax = plt.subplots(figsize=(8, 6))
    scatter = ax.scatter(
        df["tile_m"],
        df["gflops"],
        c=df["vec"],
        s=df["team"],
        cmap="viridis",
        alpha=0.8,
    )
    ax.set_xlabel("tile_m")
    ax.set_ylabel("GFLOP/s")
    ax.set_title("Tiled matmul performance (N=6144)")

    cbar = plt.colorbar(scatter, ax=ax)
    cbar.set_label("vec_len")

    # Legend for tile_n
    handles = []
    labels = []
    for t in sorted(df["tile_n"].unique()):
        handles.append(plt.Line2D([], [], marker="o", linestyle="", color="gray"))
        labels.append(f"tile_n={t}")
    ax.legend(handles, labels, title="tile_n", loc="lower left", fontsize=8)

    plt.tight_layout()
    plt.savefig("tiled_perf.png", dpi=150)

    print("Top 10 configs by GFLOP/s:")
    print(df.sort_values("gflops", ascending=False).head(10))


if __name__ == "__main__":
    main()

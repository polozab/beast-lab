import pandas as pd
import matplotlib.pyplot as plt


def main():
    df = pd.read_csv(
        "tiled_best_vs_n.txt",
        delim_whitespace=True,
        skiprows=2,
        names=["N", "tile_m", "tile_n", "team", "vec", "time_s", "gflops"],
    )
    df["config"] = df.apply(
        lambda r: f"m{int(r.tile_m)}_n{int(r.tile_n)}_t{int(r.team)}_v{int(r.vec)}",
        axis=1,
    )
    configs = list(df["config"].unique())

    fig, ax = plt.subplots(figsize=(8, 5))
    for cfg, color in zip(configs, plt.cm.tab10.colors):
        sub = df[df["config"] == cfg].sort_values("N")
        ax.plot(sub["N"], sub["gflops"], marker="o", color=color, label=cfg)

    ax.set_xlabel("N")
    ax.set_ylabel("GFLOP/s")
    ax.set_title("Best tiled configs vs N")
    ax.legend(title="config", fontsize=8)
    ax.grid(True, linestyle="--", alpha=0.3)

    plt.tight_layout()
    plt.savefig("tiled_best_vs_n.png", dpi=150)
    print("Saved plot to tiled_best_vs_n.png")


if __name__ == "__main__":
    main()

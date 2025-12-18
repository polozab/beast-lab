import matplotlib.pyplot as plt
import os


def load_rows(path):
    rows = []
    with open(path) as f:
        for line in f:
            if line.startswith("RLR compare") or line.startswith("N ") or not line.strip():
                continue
            parts = line.split()
            if len(parts) != 4:
                continue
            N, variant, time_s, gflops = parts
            rows.append(
                {
                    "N": int(N),
                    "variant": variant,
                    "time_s": float(time_s),
                    "gflops": float(gflops),
                }
            )
    return rows


def main():
    rows = load_rows("rlr_compare.txt") if os.path.exists("rlr_compare.txt") else load_rows("rlr_compare_6144.txt")
    variants = sorted({r["variant"] for r in rows})
    fig, ax = plt.subplots(figsize=(8, 5))
    colors = {"kokkos_mm_TT_RLR": "tab:blue", "kokkos_mm_TT_RLR_dual_tiled": "tab:orange"}

    for variant in variants:
        sub = sorted([r for r in rows if r["variant"] == variant], key=lambda x: x["N"])
        ax.plot(
            [r["N"] for r in sub],
            [r["gflops"] for r in sub],
            marker="o",
            color=colors.get(variant, "gray"),
            label=variant,
        )

    ax.set_xlabel("N")
    ax.set_ylabel("GFLOP/s")
    ax.set_title("RLR vs RLR_dual_tiled")
    ax.grid(True, linestyle="--", alpha=0.3)
    ax.legend()
    plt.tight_layout()
    plt.savefig("rlr_compare.png", dpi=150)
    print("Saved rlr_compare.png")


if __name__ == "__main__":
    main()

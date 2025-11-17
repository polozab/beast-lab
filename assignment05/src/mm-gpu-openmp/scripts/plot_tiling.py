#!/usr/bin/env python3
"""
Visualize tiling performance (row vs. column layouts) from data/t2/tiling_*.csv.

Creates:
  - data/t2/heatmap_tiling_col.png : MFLOPs heatmap (N rows, tile size cols) for column-major
  - data/t2/heatmap_tiling_row.png : MFLOPs heatmap (N rows, tile size cols) for row-major
  - data/t2/tiling_best.png        : per-N best MFLOPs with the winning tile size annotated

Expected CSV columns: N, Padded_N, Tiles, REP, Teams, Threads, Actual_Teams, Actual_Threads, MFLOPS
Tile size is parsed from the filename suffix (e.g., tiling_col_32.csv -> tile=32, variant=col).
"""

from pathlib import Path
import re
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt


DATA_DIR = Path(__file__).resolve().parents[1] / "data" / "t2"


def load_tiling_data():
    rows = []
    for csv in sorted(DATA_DIR.glob("tiling_*.csv")):
        m = re.match(r"tiling_(row|col)_(\d+)\.csv", csv.name)
        if not m:
            print(f"Skip {csv.name}: pattern not matched")
            continue
        variant = m.group(1)
        tile = int(m.group(2))
        try:
            df = pd.read_csv(csv)
        except Exception as exc:
            print(f"Skip {csv.name}: {exc}")
            continue

        df.rename(columns=lambda c: c.strip(), inplace=True)
        if df.empty or "N" not in df.columns or "MFLOPS" not in df.columns:
            print(f"Skip {csv.name}: missing columns")
            continue

        df["variant"] = variant
        df["tile"] = tile
        rows.append(df[["N", "MFLOPS", "variant", "tile"]])

    if not rows:
        raise RuntimeError(f"No tiling CSVs found in {DATA_DIR}")

    return pd.concat(rows, ignore_index=True)


def plot_heatmaps(df: pd.DataFrame):
    for variant in ["col", "row"]:
        sub = df[df["variant"] == variant]
        pivot = sub.pivot_table(index="N", columns="tile", values="MFLOPS", aggfunc="mean")
        pivot = pivot.sort_index().sort_index(axis=1)

        # Larger figure so annotations fit inside cells
        plt.figure(figsize=(14, 8))
        sns.heatmap(
            pivot,
            annot=False,  # omit per-cell numbers
            cmap="viridis",
            cbar_kws={"label": "MFLOPs"},
        )
        plt.xlabel("Tile size")
        plt.ylabel("Matrix size N")
        plt.yticks(rotation=0)
        plt.tight_layout()
        out = DATA_DIR / f"heatmap_tiling_{variant}.png"
        plt.savefig(out, dpi=200)
        print(f"Wrote {out}")
        plt.close()


def plot_best_per_n(df: pd.DataFrame):
    best = df.sort_values("MFLOPS", ascending=False).groupby(["N", "variant"]).head(1)
    plt.figure(figsize=(10, 6))
    for variant, marker in [("col", "o"), ("row", "s")]:
        sub = best[best["variant"] == variant].sort_values("N")
        plt.plot(sub["N"], sub["MFLOPS"], marker=marker, label=f"{variant} best")
        for _, r in sub.iterrows():
            plt.text(r["N"], r["MFLOPS"], f"{int(r['tile'])}", fontsize=8, ha="center", va="bottom")

    plt.xlabel("Matrix size N")
    plt.ylabel("Best MFLOPs (tile annotated)")
    plt.legend(title="Layout")
    plt.tight_layout()
    out = DATA_DIR / "tiling_best.png"
    plt.savefig(out, dpi=200)
    print(f"Wrote {out}")
    plt.close()


def main():
    df = load_tiling_data()
    plot_heatmaps(df)
    plot_best_per_n(df)


if __name__ == "__main__":
    main()

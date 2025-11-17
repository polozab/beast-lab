#!/usr/bin/env python3
"""
Build a heatmap of MFLOP/s over (threads-per-team, teams) from all CSVs in data/.

Expected CSV columns: N, REP, Teams, Threads, Actual_Teams, Actual_Threads, MFLOPS
The script pivots by Threads (rows) and Teams (columns) and writes data/heatmap.png.
"""

from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns


REQUIRED_COLS = {"Threads", "Teams", "MFLOPS"}


def load_data(data_dir: Path) -> pd.DataFrame:
    frames = []
    for csv in sorted(data_dir.glob("*.csv")):
        try:
            df = pd.read_csv(csv)
        except Exception as exc:
            print(f"Skip {csv.name}: {exc}")
            continue

        # Normalize headers that may contain leading/trailing spaces
        df.rename(columns=lambda c: c.strip(), inplace=True)

        # Skip empty or malformed files
        if df.empty or not REQUIRED_COLS.issubset(df.columns):
            print(f"Skip {csv.name}: missing required columns {REQUIRED_COLS}")
            continue

        df = df.dropna(subset=["MFLOPS"])
        if df.empty:
            print(f"Skip {csv.name}: no MFLOPS data")
            continue

        frames.append(df)

    if not frames:
        raise RuntimeError(f"No valid CSV files found in {data_dir}")

    return pd.concat(frames, ignore_index=True)


def build_heatmap(df: pd.DataFrame, out_path: Path) -> None:
    # Pivot to Threads (rows) × Teams (cols)
    pivot = df.pivot_table(
        index="Threads",
        columns="Teams",
        values="MFLOPS",
        aggfunc="mean",
    ).sort_index().sort_index(axis=1)

    plt.figure(figsize=(10, 6))
    sns.heatmap(
        pivot,
        annot=True,
        fmt=".0f",
        cmap="viridis",
        cbar_kws={"label": "MFLOPs"},
    )
    plt.xlabel("Teams")
    plt.ylabel("Threads per Team")
    plt.gca().invert_yaxis()  # so rows go 8 -> 1024 top-to-bottom
    plt.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(out_path, dpi=200)
    print(f"Wrote heatmap to {out_path}")


def main():
    repo_root = Path(__file__).resolve().parents[1]
    data_dir = repo_root / "data"
    out_path = data_dir / "heatmap.png"

    df = load_data(data_dir)
    build_heatmap(df, out_path)


if __name__ == "__main__":
    main()

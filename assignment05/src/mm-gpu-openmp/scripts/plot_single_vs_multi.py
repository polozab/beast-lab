#!/usr/bin/env python3
"""
Plot single-GPU MFLOPs vs multi-GPU best MFLOPs and the speedup.

Inputs:
  - Hardcoded single-GPU results (below)
  - data/multi_summary.csv for max MFLOPs per N across multi runs

Outputs:
  - data/single_vs_multi.png : line chart of single vs best multi
  - data/speedup.png         : speedup (best multi / single)
"""

from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt


SINGLE_DATA = [
    (100, 35412.135463),
    (200, 152474.626983),
    (300, 309602.327659),
    (400, 335496.807211),
    (500, 347253.209748),
    (750, 343189.547319),
    (1000, 344333.102104),
    (1250, 349971.575589),
    (1500, 339913.698849),
    (1750, 345386.003881),
    (2000, 371263.047445),
    (2500, 334639.469555),
    (3000, 330004.643715),
    (3500, 325264.396712),
    (4000, 343479.886612),
]


def load_multi_best(data_dir: Path) -> pd.DataFrame:
    summary_path = data_dir / "multi_summary.csv"
    df = pd.read_csv(summary_path)
    # Ensure numeric N and max
    df["N"] = df["N"].astype(int)
    df["max"] = df["max"].astype(float)
    return df[["N", "max"]]


def plot_single_multi(single_df: pd.DataFrame, multi_df: pd.DataFrame, data_dir: Path):
    merged = pd.merge(single_df, multi_df, on="N", how="inner", suffixes=("_single", "_multi"))

    plt.figure(figsize=(12, 6))
    plt.plot(merged["N"], merged["MFLOPs_single"], "o-", label="Single GPU")
    plt.plot(merged["N"], merged["max"], "o-", label="Best Multi-GPU")
    plt.xlabel("Matrix size N")
    plt.ylabel("MFLOPs")
    plt.xticks(rotation=45)
    plt.legend()
    plt.tight_layout()
    out = data_dir / "single_vs_multi.png"
    plt.savefig(out, dpi=200)
    print(f"Wrote {out}")

    # Speedup
    speedup = merged["max"] / merged["MFLOPs_single"]
    plt.figure(figsize=(12, 4))
    plt.plot(merged["N"], speedup, "o-", color="darkgreen")
    plt.xlabel("Matrix size N")
    plt.ylabel("Speedup (multi / single)")
    plt.xticks(rotation=45)
    plt.tight_layout()
    out2 = data_dir / "speedup.png"
    plt.savefig(out2, dpi=200)
    print(f"Wrote {out2}")


def main():
    repo = Path(__file__).resolve().parents[1]
    data_dir = repo / "data"

    single_df = pd.DataFrame(SINGLE_DATA, columns=["N", "MFLOPs_single"])
    multi_df = load_multi_best(data_dir)
    plot_single_multi(single_df, multi_df, data_dir)


if __name__ == "__main__":
    main()

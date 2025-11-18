#!/usr/bin/env python3
"""
Aggregate repeated multi-GPU runs (multi_*.csv) and visualize variability.

Outputs:
  - data/multi_summary.csv : count/mean/median/min/max/std of MFLOPs per N
  - data/multi_best.csv    : best MFLOPs per N with run identifier
  - data/multi_boxplot.png : boxplot of MFLOPs per N across runs

Expected CSV columns: N, REP, devices, MFLOPs (may appear as "N=100" etc.).
"""

from pathlib import Path
import re
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt


def normalize_columns(df: pd.DataFrame) -> pd.DataFrame:
    # Strip whitespace and split on '=' if present
    df.columns = [c.strip() for c in df.columns]
    out = pd.DataFrame()
    for col in ["N", "REP", "devices", "MFLOPs"]:
        raw = df[col]
        if raw.dtype == object:
            out[col] = raw.astype(str).str.replace(f"{col}=", "", regex=False).str.strip().astype(float)
        else:
            out[col] = raw.astype(float)
    out["N"] = out["N"].astype(int)
    out["REP"] = out["REP"].astype(int)
    out["devices"] = out["devices"].astype(int)
    return out


def load_runs(data_dir: Path) -> pd.DataFrame:
    frames = []
    for csv in sorted(data_dir.glob("multi_*.csv")):
        try:
            df = pd.read_csv(csv)
        except Exception as exc:
            print(f"Skip {csv.name}: {exc}")
            continue
        try:
            norm = normalize_columns(df)
        except Exception as exc:
            print(f"Skip {csv.name}: could not normalize ({exc})")
            continue
        norm["run"] = csv.stem
        frames.append(norm)
    if not frames:
        raise RuntimeError(f"No multi_*.csv files found in {data_dir}")
    return pd.concat(frames, ignore_index=True)


def write_summary(df: pd.DataFrame, data_dir: Path):
    # Keep only the most useful stats
    agg = df.groupby("N")["MFLOPs"].agg(["count", "mean", "min", "max"]).reset_index()
    agg.to_csv(data_dir / "multi_summary.csv", index=False)
    best = df.loc[df.groupby("N")["MFLOPs"].idxmax()][["N", "MFLOPs", "run"]]
    best.to_csv(data_dir / "multi_best.csv", index=False)
    explain = """| Metric | Meaning |
| ------ | ------- |
| count  | Number of runs collected for this N |
| mean   | Average MFLOPs across runs |
| min    | Slowest observed MFLOPs |
| max    | Fastest observed MFLOPs |
"""
    (data_dir / "multi_summary_explained.md").write_text(explain)
    print(f"Wrote {data_dir / 'multi_summary.csv'}, {data_dir / 'multi_best.csv'}, "
          f"and {data_dir / 'multi_summary_explained.md'}")


def plot_mean_with_range(df: pd.DataFrame, data_dir: Path):
    # Convert to GFLOPs for plotting
    df = df.copy()
    df["GFLOPs"] = df["MFLOPs"] / 1000.0
    agg = df.groupby("N")["GFLOPs"].agg(["mean", "min", "max"]).reset_index()
    plt.figure(figsize=(12, 6))
    plt.errorbar(
        agg["N"],
        agg["mean"],
        yerr=[agg["mean"] - agg["min"], agg["max"] - agg["mean"]],
        fmt="o-",
        color="steelblue",
        ecolor="gray",
        capsize=4,
    )
    plt.xlabel("Matrix size N")
    plt.ylabel("GFLOPs (mean with min/max range)")
    plt.xticks(rotation=45)
    plt.tight_layout()
    out = data_dir / "multi_mean_range.png"
    plt.savefig(out, dpi=200)
    print(f"Wrote {out}")


def main():
    repo = Path(__file__).resolve().parents[1]
    data_dir = repo / "data"
    df = load_runs(data_dir)
    write_summary(df, data_dir)
    plot_mean_with_range(df, data_dir)


if __name__ == "__main__":
    main()

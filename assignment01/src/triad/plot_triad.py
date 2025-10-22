#!/usr/bin/env python3
import argparse
import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import LogLocator, LogFormatterMathtext

def load_ngflops(path, use_bytes_per_element=4):
    """
    Load only the first two columns (N and GFLOPS) from a CSV with header.
    If the CSV has named columns, we read by name; otherwise we take the first two columns.
    Returns (x_kib, gflops) where x is N converted to KiB: N * bytes_per_element / 1024.
    """
    # Try by column names first
    try:
        df = pd.read_csv(path, comment="#")
        # Normalize possible variations in header casing / spacing
        cols = {c.strip().lower(): c for c in df.columns}
        if "n" in cols and "gflops" in cols:
            n = df[cols["n"]].to_numpy(dtype=float)
            gflops = df[cols["gflops"]].to_numpy(dtype=float)
        else:
            # Fallback: take first two columns positionally
            if df.shape[1] < 2:
                raise ValueError("Expected at least two columns for N and GFLOPS.")
            n = df.iloc[:, 0].to_numpy(dtype=float)
            gflops = df.iloc[:, 1].to_numpy(dtype=float)
    except Exception as e:
        raise RuntimeError(f"Failed to read {path}: {e}")

    x_kib = 4 * n * use_bytes_per_element / (2**10)
    return x_kib, gflops

def main():
    ap = argparse.ArgumentParser(description="Plot Triad GFLOP/s vs. vector size from CSV(s).")
    ap.add_argument("csv", nargs="+", help="Input CSV file(s). Only first two columns (N, GFLOPS) are used.")
    ap.add_argument("--title", default="Triad on Sapphire Rapids",
                    help="Plot title.")
    ap.add_argument("--bytes-per-element", type=int, default=4,
                    help="Bytes per element for N→KiB conversion (default: 4 for float).")
    ap.add_argument("--marker", default="o", help="Matplotlib marker for lines (default: o).")
    args = ap.parse_args()

    fig, ax = plt.subplots(figsize=(8, 5))

    any_plotted = False
    for path in args.csv:
        try:
            x_kib, gflops = load_ngflops(path, use_bytes_per_element=args.bytes_per_element)
            label = os.path.basename(path)
            ax.plot(x_kib, gflops, marker=args.marker, label=label)
            any_plotted = True
        except Exception as e:
            print(e)

    if not any_plotted:
        raise SystemExit("No valid data plotted. Check your CSV files.")

    ax.set_ylabel("GFLOP/s")
    ax.set_xlabel("∑ Vector size [KiB]")
    ax.grid(which="both", alpha=0.6)

    # Log-scaled X with 10^x ticks (as in your original)
    ax.set_xscale("log")
    ax.xaxis.set_major_locator(LogLocator(base=10))
    ax.xaxis.set_major_formatter(LogFormatterMathtext())

    # Secondary Y axis: Bandwidth [GB/s] = 20 * GFLOPS  (as in your example)
    def gflops_to_gbs(y):
        return 20.0 * y
    def gbs_to_gflops(y):
        return y / 20.0
    ax2 = ax.secondary_yaxis("right", functions=(gflops_to_gbs, gbs_to_gflops))
    ax2.set_ylabel("Bandwidth [GB/s]")

    ax.legend(loc="best")
    plt.title(args.title)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
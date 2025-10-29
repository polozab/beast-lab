#!/usr/bin/env python3
import argparse
import os
import re
import numpy as np
import matplotlib.pyplot as plt

LINE_PATTERN = re.compile(r"MFLOPS\(N=\s*([0-9]+)\s*\):\s*([0-9]+(?:\.[0-9]+)?)")

def load_nmflops(path):
    """
    Parse a text file that contains lines with the format:
        Overhead: <value>, MFLOPS(N=<n>): <mflops>
    Returns (n_values, mflops_values) as numpy arrays.
    """
    n_values = []
    mflops_values = []

    try:
        with open(path, "r", encoding="utf-8") as fh:
            for line in fh:
                match = LINE_PATTERN.search(line)
                if not match:
                    continue
                n_values.append(float(match.group(1)))
                mflops_values.append(float(match.group(2)))
    except OSError as exc:
        raise RuntimeError(f"Failed to read {path}: {exc}") from exc

    if not n_values:
        raise RuntimeError(f"No MFLOPS entries found in {path}.")

    return np.array(n_values), np.array(mflops_values)

def main():
    ap = argparse.ArgumentParser(description="Plot MFLOP/s vs. N from MFLOPS text file(s).")
    ap.add_argument("inputs", nargs="+", help="Input text file(s) matching 'Overhead: ..., MFLOPS(N= ...): ...'.")
    ap.add_argument("--title", default="Matrix Multiplication MFLOPS",
                    help="Plot title.")
    ap.add_argument("--marker", default="o", help="Matplotlib marker for lines (default: o).")
    args = ap.parse_args()

    fig, ax = plt.subplots(figsize=(8, 5))

    any_plotted = False
    for path in args.inputs:
        try:
            n_values, mflops_values = load_nmflops(path)
            label = os.path.basename(path)
            ax.plot(n_values, mflops_values, marker=args.marker, label=label)
            any_plotted = True
        except Exception as e:
            print(e)

    if not any_plotted:
        raise SystemExit("No valid data plotted. Check your text input files.")

    ax.set_ylabel("MFLOP/s")
    ax.set_xlabel("N")
    ax.grid(alpha=0.6)

    ax.legend(loc="best")
    plt.title(args.title)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()

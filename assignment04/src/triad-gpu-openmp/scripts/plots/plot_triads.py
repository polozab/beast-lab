#!/usr/bin/env python3
"""
Plot MFLOPS vs. vector size for one or multiple triad benchmark CSV files.

Example:
    python plot_triads.py ../../report/task3-measurements/*.csv --title "GH100 triad" \
        --marker o --xlog --output gh100-triad.png
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
from typing import Iterable, List, Tuple

import matplotlib.pyplot as plt


def read_curve(path: Path) -> Tuple[List[float], List[float]]:
    x_vals: List[float] = []
    y_vals: List[float] = []
    with path.open(newline="") as handle:
        reader = csv.DictReader(handle, skipinitialspace=True)
        for row in reader:
            try:
                x_vals.append(float(row["Data size"]))
                y_vals.append(float(row["MFLOPS"]))
            except KeyError as exc:
                raise ValueError(f"{path} is missing required columns: {exc}") from exc
            except ValueError as exc:
                raise ValueError(f"{path} contains non-numeric values in row {row}") from exc
    if not x_vals:
        raise ValueError(f"{path} did not provide any usable rows.")
    return x_vals, y_vals


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot vector size vs. MFLOPS from triad benchmark CSV files."
    )
    parser.add_argument("inputs", nargs="+", help="CSV files to plot (glob patterns allowed).")
    parser.add_argument("--title", default="Triad performance", help="Plot title.")
    parser.add_argument(
        "--marker", default="o", help="Matplotlib marker for each curve (default: o)."
    )
    parser.add_argument(
        "--xlog",
        action="store_true",
        help="Use logarithmic x-axis (useful for powers-of-two sweeps).",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Optional file to save the figure (PNG, PDF, ...). If omitted, display interactively.",
    )
    return parser.parse_args()


def expand_files(patterns: Iterable[str]) -> List[Path]:
    files: List[Path] = []
    for pattern in patterns:
        matched = list(Path().glob(pattern))
        if matched:
            files.extend(sorted(matched))
        else:
            files.append(Path(pattern))
    return files


def main() -> None:
    args = parse_args()
    files = expand_files(args.inputs)
    if not files:
        raise SystemExit("No input files provided.")

    fig, ax = plt.subplots(figsize=(9, 5))
    any_data = False
    for file_path in files:
        if not file_path.exists():
            print(f"Warning: {file_path} not found, skipping.")
            continue
        try:
            sizes, mflops = read_curve(file_path)
        except ValueError as exc:
            print(exc)
            continue
        ax.plot(sizes, mflops, marker=args.marker, label=file_path.name)
        any_data = True

    if not any_data:
        raise SystemExit("No valid CSV data to plot.")

    ax.set_xlabel("Vector size (elements)")
    ax.set_ylabel("MFLOPS")
    ax.set_title(args.title)
    ax.grid(True, which="both", alpha=0.4)
    if args.xlog:
        ax.set_xscale("log", base=2)
    ax.legend()
    fig.tight_layout()

    if args.output:
        fig.savefig(args.output, dpi=200)
        print(f"Wrote figure to {args.output}")
    else:
        plt.show()


if __name__ == "__main__":
    main()

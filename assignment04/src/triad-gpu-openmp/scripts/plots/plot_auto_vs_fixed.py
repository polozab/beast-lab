#!/usr/bin/env python3
"""
Compare OpenMP auto-selected (teams, threads) results against fixed configurations.

The script plots MFLOPS vs. data size for the auto baseline and any number of fixed
configurations, plus a ratio subplot that highlights where fixed settings outperform
or underperform the automatic choices.
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
from typing import Dict, Iterable, List, Tuple

import matplotlib.pyplot as plt


def read_curve(path: Path) -> Tuple[List[float], List[float]]:
    sizes: List[float] = []
    mflops: List[float] = []
    with path.open(newline="") as handle:
        reader = csv.DictReader(handle, skipinitialspace=True)
        for row in reader:
            try:
                sizes.append(float(row["Data size"]))
                mflops.append(float(row["MFLOPS"]))
            except KeyError as exc:
                raise ValueError(f"{path} missing required column: {exc}") from exc
            except ValueError as exc:
                raise ValueError(f"{path} contains non-numeric values: {row}") from exc
    if not sizes:
        raise ValueError(f"{path} did not yield any datapoints.")
    return sizes, mflops


def curve_to_map(sizes: List[float], values: List[float]) -> Dict[float, float]:
    return {size: value for size, value in zip(sizes, values)}


def expand_patterns(patterns: Iterable[str]) -> List[Path]:
    files: List[Path] = []
    for pattern in patterns:
        matched = list(Path().glob(pattern))
        if matched:
            files.extend(sorted(matched))
        else:
            files.append(Path(pattern))
    return files


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot auto-selected vs. fixed team/thread MFLOPS results."
    )
    parser.add_argument(
        "--auto",
        required=True,
        type=Path,
        help="CSV containing the auto-selected configuration results (Data size, MFLOPS).",
    )
    parser.add_argument(
        "--fixed",
        nargs=2,
        action="append",
        metavar=("FILE", "LABEL"),
        help="Add a fixed configuration curve: provide CSV path/pattern and label. "
        "Repeat the flag to add multiple curves.",
    )
    parser.add_argument("--title", default="Auto vs. fixed teams/threads")
    parser.add_argument(
        "--auto-label",
        help="Custom legend label for the auto-selected curve (default: file stem + '(auto)').",
    )
    parser.add_argument(
        "--hide-auto",
        action="store_true",
        help="Do not plot the auto baseline (still requires --auto for reference).",
    )
    parser.add_argument(
        "--marker",
        default="o",
        help="Matplotlib marker style for lines (default: o).",
    )
    parser.add_argument(
        "--xlog",
        action="store_true",
        help="Use logarithmic x-axis (base 2) for the data size.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Optional destination to save the plot (PNG, PDF, ...).",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    auto_sizes, auto_mflops = read_curve(args.auto)

    fixed_pairs: List[Tuple[Path, str]] = []
    if not args.fixed:
        raise SystemExit("At least one --fixed file/label pair must be provided.")
    for file_pattern, label in args.fixed:
        matched = list(Path().glob(file_pattern))
        if matched:
            for file_path in sorted(matched):
                fixed_pairs.append((file_path, label))
        else:
            fixed_pairs.append((Path(file_pattern), label))
    auto_label = args.auto_label or f"{args.auto.stem} (auto)"

    fig, ax_main = plt.subplots(figsize=(9, 5))

    if not args.hide_auto:
        ax_main.plot(auto_sizes, auto_mflops, marker=args.marker, label=auto_label)

    for path, label in fixed_pairs:
        if not path.exists():
            print(f"Warning: {path} not found, skipping.")
            continue
        try:
            sizes, values = read_curve(path)
        except ValueError as exc:
            print(exc)
            continue
        ax_main.plot(sizes, values, marker=args.marker, label=label)

    ax_main.set_ylabel("MFLOPS")
    ax_main.set_title(args.title)
    ax_main.grid(True, which="both", alpha=0.4)
    ax_main.legend()

    ax_main.set_xlabel("Data size")

    if args.xlog:
        ax_main.set_xscale("log", base=2)

    fig.tight_layout()

    if args.output:
        fig.savefig(args.output, dpi=200)
        print(f"Wrote figure to {args.output}")
    else:
        plt.show()


if __name__ == "__main__":
    main()

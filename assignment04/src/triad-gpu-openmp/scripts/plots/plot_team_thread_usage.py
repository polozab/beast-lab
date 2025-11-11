#!/usr/bin/env python3
"""
Visualize how OpenMP chose teams/threads as a function of data size.

Example:
    python plot_team_thread_usage.py ../../t4b.csv --title "Auto-selected launch sizes" \
        --output t4b-teams-threads.png
"""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path
from typing import Iterable, List, Tuple

import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot Data size vs. Teams/Threads from triad CSV files."
    )
    parser.add_argument("inputs", nargs="+", help="CSV files containing Data size, Teams, Threads.")
    parser.add_argument(
        "--title",
        default="Runtime decided teams and threads",
        help="Plot title.",
    )
    parser.add_argument(
        "--marker",
        default="o",
        help="Marker used for the curves (default: o).",
    )
    parser.add_argument(
        "--xlog",
        action="store_true",
        help="Logarithmic x-axis (base 2) for the data sizes.",
    )
    parser.add_argument(
        "--teams-log2",
        action="store_true",
        help="Display teams as log2 values (i.e., 2^x) instead of raw counts.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Optional output path for the figure (PNG, PDF, ...).",
    )
    return parser.parse_args()


def load_columns(path: Path) -> Tuple[List[float], List[float], List[float]]:
    sizes: List[float] = []
    teams: List[float] = []
    threads: List[float] = []
    with path.open(newline="") as handle:
        reader = csv.DictReader(handle, skipinitialspace=True)
        for row in reader:
            try:
                sizes.append(float(row["Data size"]))
                teams.append(float(row["Teams"]))
                threads.append(float(row["Threads"]))
            except KeyError as exc:
                raise ValueError(f"{path} missing column: {exc}") from exc
            except ValueError as exc:
                raise ValueError(f"{path} has non-numeric values: {row}") from exc
    if not sizes:
        raise ValueError(f"{path} did not provide any rows.")
    return sizes, teams, threads


def expand_inputs(patterns: Iterable[str]) -> List[Path]:
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
    files = expand_inputs(args.inputs)
    if not files:
        raise SystemExit("No input CSV files provided.")

    fig, ax = plt.subplots(figsize=(9, 5))
    twin = ax.twinx()
    all_sizes: List[float] = []
    color_cycle = plt.rcParams.get("axes.prop_cycle", None)
    base_colors = (
        color_cycle.by_key().get("color", [])
        if color_cycle is not None
        else ["C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9"]
    )
    for idx, path in enumerate(files):
        if not path.exists():
            print(f"Warning: {path} not found, skipping.")
            continue
        try:
            sizes, teams, threads = load_columns(path)
        except ValueError as exc:
            print(exc)
            continue

        teams_plot = teams
        if args.teams_log2:
            try:
                teams_plot = [math.log2(value) for value in teams]
            except ValueError as exc:
                print(f"Cannot apply log2 to teams in {path}: {exc}")
                continue

        all_sizes.extend(sizes)

        ax.plot(
            sizes,
            teams_plot,
            marker=args.marker,
            label="Teams",
            color=base_colors[(2 * idx) % len(base_colors)],
        )
        twin.plot(
            sizes,
            threads,
            marker=args.marker,
            linestyle="--",
            label="Threads",
            color=base_colors[(2 * idx + 1) % len(base_colors)],
        )

    ax.set_xlabel("Data size")
    ax.set_ylabel("Teams")
    twin.set_ylabel("Threads")
    ax.set_title(args.title)
    ax.grid(True, which="both", alpha=0.4)

    if args.teams_log2:
        def teams_formatter(value, _pos):
            if value <= 0:
                return ""
            rounded = round(value)
            if abs(value - rounded) < 1e-6:
                return fr"$2^{{{int(rounded)}}}$"
            return f"{value:.2f}"
        ax.yaxis.set_major_formatter(FuncFormatter(teams_formatter))

    if args.xlog:
        ax.set_xscale("log", base=2)
        twin.set_xscale("log", base=2)

        def power_of_two_formatter(value, _pos):
            if value <= 0:
                return ""
            exponent = math.log2(value)
            exponent_rounded = round(exponent)
            if abs(exponent - exponent_rounded) < 1e-6:
                return fr"$2^{{{int(exponent_rounded)}}}$"
            return f"{value:.0f}"

        formatter = FuncFormatter(power_of_two_formatter)
        ax.xaxis.set_major_formatter(formatter)
        twin.xaxis.set_major_formatter(formatter)

        positive_sizes = [s for s in all_sizes if s > 0]
        if positive_sizes:
            min_exp = math.floor(math.log2(min(positive_sizes)))
            max_exp = math.ceil(math.log2(max(positive_sizes)))
            ticks = [2 ** exp for exp in range(min_exp, max_exp + 1)]
            ax.set_xticks(ticks)
            twin.set_xticks(ticks)

    # Build combined legend without duplicates
    lines = ax.get_lines() + twin.get_lines()
    seen = set()
    unique_lines = []
    for line in lines:
        label = line.get_label()
        if label in seen:
            continue
        seen.add(label)
        unique_lines.append(line)
    ax.legend(unique_lines, [line.get_label() for line in unique_lines], loc="best")

    fig.tight_layout()
    if args.output:
        fig.savefig(args.output, dpi=200)
        print(f"Wrote figure to {args.output}")
    else:
        plt.show()


if __name__ == "__main__":
    main()

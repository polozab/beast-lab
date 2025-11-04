#!/usr/bin/env python3
"""
Visualise energy efficiency (GFLOP/s per Watt) for Task 4 of assignment 03.

Each input CSV is expected to contain at least these columns (header names are
case-insensitive and may include underscores or spaces):

    - Label (default column name: "Label"; override with --label-column)
    - GFLOPS / MFLOPS / TFLOPS (auto-detected or via --gflops-column)
    - Power / Watts (auto-detected or via --power-column)

GFLOP/s values are converted automatically if the column uses MFLOPS or TFLOPS.
Power values are assumed to be in Watts.

Example:
    python plot_gflops_per_watt.py measurements.csv --output gflops_per_watt.png
    python plot_gflops_per_watt.py data/*.csv --sort-desc
"""

from __future__ import annotations

import argparse
import os
from dataclasses import dataclass
from typing import Iterable, List, Sequence, Tuple

import matplotlib.pyplot as plt
import pandas as pd


GFLOPS_CANDIDATES = (
    "gflops",
    "gf/s",
    "gigaflops",
    "performance",
    "throughput",
    "mflops",
    "tflops",
)
POWER_CANDIDATES = (
    "power",
    "power_w",
    "watts",
    "avg_power",
    "package_power",
    "socket_power",
)


@dataclass
class EfficiencyPoint:
    label: str
    gflops: float
    power_w: float

    @property
    def gflops_per_watt(self) -> float:
        return self.gflops / self.power_w if self.power_w else float("nan")


def normalise_columns(columns: Sequence[str]) -> dict[str, str]:
    """
    Build a mapping from normalised (lowercase, stripped) names to the actual
    column labels present in the CSV.
    """
    mapping: dict[str, str] = {}
    for col in columns:
        normalised = col.strip().lower().replace(" ", "_")
        mapping[normalised] = col
    return mapping


def select_column(
    mapping: dict[str, str],
    explicit: str | None,
    candidates: Iterable[str],
    kind: str,
) -> Tuple[str, str]:
    """
    Return the actual column name and its normalised key.
    """
    if explicit:
        key = explicit.strip().lower().replace(" ", "_")
        if key not in mapping:
            raise argparse.ArgumentTypeError(
                f"Column '{explicit}' not found in input data for {kind}."
            )
        return mapping[key], key

    for candidate in candidates:
        key = candidate.strip().lower().replace(" ", "_")
        if key in mapping:
            return mapping[key], key

    raise argparse.ArgumentTypeError(
        f"Unable to locate a column for {kind}. "
        f"Tried: {', '.join(candidates)}"
    )


def load_points(
    path: str,
    label_col: str | None,
    gflops_col: str | None,
    power_col: str | None,
) -> List[EfficiencyPoint]:
    df = pd.read_csv(path, comment="#")
    if df.empty:
        raise RuntimeError(f"{path}: no rows to plot.")

    mapping = normalise_columns(df.columns)

    # label column is optional; fall back to auto-generated labels.
    label_key = label_col.strip().lower().replace(" ", "_") if label_col else "label"
    label_available = label_key in mapping

    gflops_actual, gflops_key = select_column(
        mapping, gflops_col, GFLOPS_CANDIDATES, "GFLOP/s"
    )
    power_actual, _ = select_column(
        mapping, power_col, POWER_CANDIDATES, "Power (Watts)"
    )

    labels: List[str] = []
    if label_available:
        labels = df[mapping[label_key]].astype(str).tolist()
    else:
        basename = os.path.splitext(os.path.basename(path))[0]
        labels = [f"{basename}[{idx}]" for idx in range(len(df))]

    gflops_values = df[gflops_actual].astype(float)
    power_values = df[power_actual].astype(float)

    # handle unit conversions if the column is MFLOPS or TFLOPS.
    gflops_norm = gflops_values.copy()
    if "mflops" in gflops_key:
        gflops_norm = gflops_values * 1e-3
    elif "tflops" in gflops_key:
        gflops_norm = gflops_values * 1e3

    points: List[EfficiencyPoint] = []
    for label, gflops, power in zip(labels, gflops_norm, power_values, strict=True):
        points.append(EfficiencyPoint(label=label, gflops=gflops, power_w=power))
    return points


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Plot GFLOP/s per Watt for assignment 03 task 4 measurements."
    )
    parser.add_argument(
        "csv",
        nargs="*",
        help="Optional CSV file(s) with columns for label, GFLOP/s (or MFLOPS/TFLOPS), "
        "and power/watts.",
    )
    parser.add_argument(
        "--label-column",
        help="Name of the column containing labels (default: auto-detect 'Label').",
    )
    parser.add_argument(
        "--gflops-column",
        help="Explicit GFLOP/s column name. If omitted, common names are tried.",
    )
    parser.add_argument(
        "--power-column",
        help="Explicit power column name. If omitted, common names are tried.",
    )
    parser.add_argument(
        "--sort-desc",
        action="store_true",
        help="Sort bars in descending GFLOP/s per Watt order.",
    )
    parser.add_argument(
        "--title",
        default="GFLOP/s per Watt",
        help="Figure title.",
    )
    parser.add_argument(
        "--output",
        help="Optional path to save the figure (PNG, PDF, ...).",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Show the plot even if --output is specified.",
    )
    parser.add_argument(
        "--point",
        action="append",
        nargs=3,
        metavar=("LABEL", "GFLOPS", "WATTS"),
        help="Add a single measurement directly: label GFLOP/s power_watts. "
        "May be supplied multiple times.",
    )
    return parser


def main() -> None:
    parser = build_parser()
    args = parser.parse_args()

    points: List[EfficiencyPoint] = []
    if not args.csv and not args.point:
        parser.error("Provide at least one CSV file or --point entry.")

    for path in args.csv:
        resolved = os.path.abspath(path)
        if not os.path.exists(resolved):
            parser.error(f"File not found: {resolved}")
        try:
            points.extend(
                load_points(
                    resolved,
                    label_col=args.label_column,
                    gflops_col=args.gflops_column,
                    power_col=args.power_column,
                )
            )
        except Exception as exc:  # noqa: BLE001 - surface parse errors
            parser.error(f"{path}: {exc}")

    if args.point:
        for label, gflops_str, watts_str in args.point:
            try:
                gflops = float(gflops_str)
                watts = float(watts_str)
            except ValueError as exc:
                parser.error(
                    f"Invalid --point values '{label} {gflops_str} {watts_str}': {exc}"
                )
            points.append(EfficiencyPoint(label=label, gflops=gflops, power_w=watts))

    if not points:
        parser.error("No data points were loaded.")

    if args.sort_desc:
        points.sort(key=lambda p: p.gflops_per_watt, reverse=True)

    labels = [pt.label for pt in points]
    ratios = [pt.gflops_per_watt for pt in points]
    gflops = [pt.gflops for pt in points]
    power = [pt.power_w for pt in points]

    fig, ax = plt.subplots(figsize=(8, 6))
    ax.scatter(power, gflops, color="#4e79a7", s=80, edgecolor="black")

    for idx, label in enumerate(labels):
        annotation = f"{label}\n{ratios[idx]:.2f} GF/W"
        ax.annotate(
            annotation,
            xy=(power[idx], gflops[idx]),
            xytext=(6, 6),
            textcoords="offset points",
            fontsize=9,
        )

    ax.set_xlabel("Power [W]")
    ax.set_ylabel("Performance [GFLOP/s]")
    ax.set_title(args.title)
    ax.grid(alpha=0.6, linestyle="--")
    fig.tight_layout()

    if args.output:
        fig.savefig(args.output, dpi=300)
    if args.show or not args.output:
        plt.show()


if __name__ == "__main__":
    main()

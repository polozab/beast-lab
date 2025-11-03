#!/usr/bin/env python3
import argparse
from dataclasses import dataclass
from typing import Iterable, List

import matplotlib.pyplot as plt
import numpy as np


@dataclass
class KernelPoint:
    label: str
    intensity: float  # FLOPs per byte
    gflops: float


def parse_points(raw_points: Iterable[List[str]]) -> List[KernelPoint]:
    points: List[KernelPoint] = []
    for label, ai_str, gflops_str in raw_points:
        try:
            ai = float(ai_str)
            gflops = float(gflops_str)
        except ValueError as exc:
            raise argparse.ArgumentTypeError(
                f"Could not parse arithmetic intensity / GFLOP/s for '{label}': {exc}"
            ) from exc
        points.append(KernelPoint(label, ai, gflops))
    return points


def build_roofline(ai_values: np.ndarray, peak_gflops: float, bandwidth: float) -> np.ndarray:
    """
    Compute the roofline curve for the supplied arithmetic intensity grid.
    Bandwidth is expected in GB/s (base-10), intensities in FLOPs/byte.
    """
    memory_bound = bandwidth * ai_values
    compute_bound = np.full_like(ai_values, peak_gflops)
    return np.minimum(memory_bound, compute_bound)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Plot a roofline model for measured kernels."
    )
    parser.add_argument(
        "--peak-gflops",
        type=float,
        required=True,
        help="Theoretical compute ceiling in GFLOP/s.",
    )
    parser.add_argument(
        "--bandwidth",
        type=float,
        required=True,
        help="Sustained memory bandwidth in GB/s.",
    )
    parser.add_argument(
        "--point",
        nargs=3,
        metavar=("LABEL", "AI", "GFLOPS"),
        action="append",
        default=[],
        help="Measured kernel (label, arithmetic intensity [FLOPs/byte], GFLOP/s). "
        "May be supplied multiple times.",
    )
    parser.add_argument(
        "--default-triad",
        action="store_true",
        help="Add a default TRIAD point with AI=0.0625 and GFLOP/s computed from "
        "bandwidth (GFLOPS = bandwidth * AI).",
    )
    parser.add_argument(
        "--output",
        help="Optional output path for saving the figure (PNG, PDF, ...). "
        "If omitted the plot window is shown.",
    )
    parser.add_argument(
        "--title",
        default="Sapphire Rapids Roofline",
        help="Figure title.",
    )

    args = parser.parse_args()

    points = parse_points(args.point)

    if args.default_triad:
        triad_gflops = args.bandwidth * 0.0625
        points.append(KernelPoint("triad", 0.0625, triad_gflops))

    if not points:
        parser.error("At least one --point is required (or use --default-triad).")

    ai_values = [p.intensity for p in points]
    gflops_values = [p.gflops for p in points]

    ai_min = min(ai_values)
    ai_max = max(ai_values + [args.peak_gflops / args.bandwidth])

    # padding on log axis
    x_min = ai_min / 5.0 if ai_min > 0 else 1e-3
    x_max = ai_max * 5.0

    ai_grid = np.geomspace(x_min, x_max, 512)
    roofline = build_roofline(ai_grid, args.peak_gflops, args.bandwidth)

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.loglog(ai_grid, roofline, label="Roofline", color="black", linewidth=2)

    for point in points:
        ax.scatter(point.intensity, point.gflops, s=60, label=point.label)
        ax.text(
            point.intensity * 1.05,
            point.gflops * 1.05,
            f"{point.gflops:.2f} GFLOP/s",
            fontsize=9,
        )

    knee_ai = args.peak_gflops / args.bandwidth
   # ax.axvline(knee_ai, color="grey", linestyle="--", linewidth=1, label="Roofline knee")

    ax.set_xlabel("Arithmetic intensity [FLOPs / byte]")
    ax.set_ylabel("Performance [GFLOP/s]")
    ax.set_title(args.title)
    ax.grid(which="both", linestyle="--", linewidth=0.1)
    ax.legend(bbox_to_anchor=(1, 0.5))

    fig.tight_layout()

    if args.output:
        fig.savefig(args.output, dpi=200)
    else:
        plt.show()


if __name__ == "__main__":
    main()

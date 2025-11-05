#!/usr/bin/env python3
"""
Plot GFLOP/W measurements for multiple devices across power states.
"""
import argparse
from collections import OrderedDict
from dataclasses import dataclass
from typing import Iterable, List, Optional, Sequence, Tuple

import matplotlib.pyplot as plt
import numpy as np


@dataclass
class EfficiencyRun:
    device: str
    state: str
    gflops_per_watt: float


DEFAULT_RUN_ARGUMENTS: Tuple[Tuple[str, str, str], ...] = (
    ("milan2", "triad", "0.16619105199516324"),
    ("milan2", "mm_small_N", "0.01954478860523322"),
    ("milan2", "mm_large_N", "2.3278145716516265"),
    ("cs2", "triad", "0.014810344827586207"),
    ("cs2", "mm_small_N", "0.003686667206866953"),
    ("cs2", "mm_large_N", "1.9140701352752372"),
)


def parse_runs(raw_runs: Iterable[Sequence[str]]) -> List[EfficiencyRun]:
    parsed: List[EfficiencyRun] = []
    for device, state, value_str in raw_runs:
        try:
            gflops_per_watt = float(value_str)
        except ValueError as exc:
            raise argparse.ArgumentTypeError(
                f"Could not parse GFLOP/W value for '{device} {state}': {exc}"
            ) from exc
        parsed.append(EfficiencyRun(device=device, state=state, gflops_per_watt=gflops_per_watt))
    return parsed


def collect_orders(runs: Sequence[EfficiencyRun]) -> Tuple[List[str], OrderedDict[str, dict]]:
    """
    Build ordered collections of devices and states to keep CLI order intact.
    """
    devices: List[str] = []
    states: OrderedDict[str, dict[str, float]] = OrderedDict()

    for run in runs:
        if run.device not in devices:
            devices.append(run.device)
        if run.state not in states:
            states[run.state] = {}
        states[run.state][run.device] = run.gflops_per_watt

    return devices, states


def compute_bar_positions(num_states: int, num_devices: int) -> Tuple[np.ndarray, float, np.ndarray]:
    x_positions = np.arange(num_states)
    # Spread bars inside each state; keep total width capped for readability.
    width = min(0.8 / max(num_devices, 1), 0.25) if num_devices > 1 else 0.5
    offsets = (np.arange(num_devices) - (num_devices - 1) / 2.0) * width
    return x_positions, width, offsets


def format_value(value: float) -> str:
    """
    Format GFLOP/W values so small numbers remain distinguishable on annotations.
    """
    abs_value = abs(value)
    if abs_value >= 10:
        formatted = f"{value:.1f}"
    elif abs_value >= 1:
        formatted = f"{value:.2f}"
    elif abs_value >= 0.1:
        formatted = f"{value:.3f}"
    elif abs_value >= 0.01:
        formatted = f"{value:.4f}"
    elif abs_value >= 0.001:
        formatted = f"{value:.5f}"
    else:
        return f"{value:.2e}"

    stripped = formatted.rstrip("0").rstrip(".")
    return stripped if stripped else "0"


def plot_runs(
    runs: Sequence[EfficiencyRun],
    title: str,
    output: Optional[str],
    annotate: bool,
    ymax_padding: float,
    y_scale: str,
) -> None:
    devices, states = collect_orders(runs)

    state_labels = list(states.keys())
    num_states = len(state_labels)
    num_devices = len(devices)

    x_positions, bar_width, offsets = compute_bar_positions(num_states, num_devices)

    fig, ax = plt.subplots(figsize=(8, 5))

    ymax = 0.0
    min_positive = np.inf

    for device_idx, device in enumerate(devices):
        values = []
        for state in state_labels:
            value = states[state].get(device)
            values.append(value if value is not None else np.nan)

        values_array = np.array(values, dtype=float)
        if y_scale == "log":
            if np.any(values_array <= 0):
                raise ValueError("Logarithmic y-scale requires all GFLOP/W values to be > 0.")
        positive_values = values_array[values_array > 0]
        if positive_values.size:
            min_positive = min(min_positive, float(np.nanmin(positive_values)))
        try:
            series_max = float(np.nanmax(values_array))
        except ValueError:
            series_max = 0.0
        ymax = max(ymax, series_max)

        bar_positions = x_positions + offsets[device_idx]
        bars = ax.bar(bar_positions, values_array, bar_width, label=device)

        if annotate:
            for rect, value in zip(bars, values_array):
                if np.isnan(value):
                    continue
                height = rect.get_height()
                if y_scale == "log":
                    text_y = height * (1.0 + ymax_padding)
                else:
                    text_y = height * 1.01
                ax.text(
                    rect.get_x() + rect.get_width() / 2.0,
                    text_y,
                    format_value(float(value)),
                    ha="center",
                    va="bottom",
                    fontsize=9,
                )

    ax.set_xticks(x_positions)
    ax.set_xticklabels(state_labels)
    ax.set_ylabel("GFlops/W")
    ax.set_title(title)
    ax.grid(axis="y", linestyle="--", linewidth=0.3)
    ax.legend(title="Device")

    ax.set_yscale(y_scale)

    if ymax > 0:
        upper = ymax * (1.0 + ymax_padding)
        if y_scale == "log":
            if not np.isfinite(min_positive) or min_positive <= 0:
                raise ValueError("Logarithmic y-scale requires all GFLOP/W values to be > 0.")
            lower = min_positive / (1.0 + ymax_padding)
            ax.set_ylim(lower, upper)
        else:
            ax.set_ylim(0, upper)

    fig.tight_layout()

    if output:
        fig.savefig(output, dpi=200)
    else:
        plt.show()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Plot GFLOP/W measurements for several devices across power states."
    )
    parser.add_argument(
        "--run",
        nargs=3,
        metavar=("DEVICE", "STATE", "GFLOP_PER_WATT"),
        action="append",
        help="Measured run supplied as (device, state, GFLOP/W). Provide multiple --run entries "
        "to override the default data set.",
    )
    parser.add_argument(
        "--title",
        default="",
        help="Figure title.",
    )
    parser.add_argument(
        "--output",
        help="Optional output path for saving the figure (PNG, PDF, ...). If omitted the plot "
        "window is shown.",
    )
    parser.add_argument(
        "--no-annotations",
        action="store_true",
        help="Hide the GFLOP/W value on top of each bar.",
    )
    parser.add_argument(
        "--ymax-padding",
        type=float,
        default=0.05,
        help="Relative headroom above the tallest bar (default: 5%%).",
    )
    parser.add_argument(
        "--y-scale",
        choices=("linear", "log"),
        default="log",
        help="Y-axis scale (default: log).",
    )

    args = parser.parse_args()

    raw_runs = args.run if args.run else DEFAULT_RUN_ARGUMENTS
    runs = parse_runs(raw_runs)
    try:
        plot_runs(
            runs=runs,
            title=args.title,
            output=args.output,
            annotate=not args.no_annotations,
            ymax_padding=args.ymax_padding,
            y_scale=args.y_scale,
        )
    except ValueError as exc:
        parser.error(str(exc))


if __name__ == "__main__":
    main()

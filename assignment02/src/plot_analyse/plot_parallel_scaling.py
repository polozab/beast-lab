#!/usr/bin/env python3
"""
Plot speedup curves for parallel matrix-multiplication experiments.

Each input dataset is expected to be a CSV-formatted text file with columns:
    N, N_padded, Tiles, Threads, Repetitions, MFLOPS, Speedup

Use --case binding:label:path to register the four required scenarios, e.g.:
    python plot_parallel_scaling.py \
        --case close:"Small N (close)" new_data/parallel/close_16.txt \
        --case close:"Large N (close)" new_data/parallel/close_16384.txt \
        --case spread:"Small N (spread)" new_data/parallel/spread_16.txt \
        --case spread:"Large N (spread)" new_data/parallel/spread_16384.txt \
        --output parallel_speedup.png
"""

import argparse
import csv
import glob
import os
from dataclasses import dataclass
from typing import List, Optional

import matplotlib.pyplot as plt


@dataclass
class ScalingCase:
    binding: str
    label: str
    path: str
    n_value: int
    threads: List[int]
    speedup: List[float]


def load_scaling_case(binding: str, label: str, path: str) -> ScalingCase:
    threads: List[int] = []
    speedup: List[float] = []
    n_value: Optional[int] = None

    with open(path, "r", encoding="utf-8") as fh:
        reader = csv.reader(fh)
        try:
            header = next(reader)
        except StopIteration as exc:
            raise RuntimeError(f"File {path} is empty") from exc

        normalized_header = [col.strip() for col in header]
        try:
            idx_n = normalized_header.index("N")
            idx_threads = normalized_header.index("Threads")
            idx_speedup = normalized_header.index("Speedup")
        except ValueError as exc:
            raise RuntimeError(
                f"File {path} must contain columns 'N', 'Threads', and 'Speedup'."
            ) from exc

        for row in reader:
            if not row:
                continue
            values = [col.strip() for col in row]
            if n_value is None:
                n_value = int(values[idx_n])
            threads.append(int(values[idx_threads]))
            speedup.append(float(values[idx_speedup]))

    if n_value is None:
        raise RuntimeError(f"No data rows found in {path}")

    # Ensure the data is sorted by thread count for clean plotting.
    combined = sorted(zip(threads, speedup), key=lambda pair: pair[0])
    sorted_threads, sorted_speedup = zip(*combined)

    return ScalingCase(
        binding=binding,
        label=label,
        path=path,
        n_value=n_value,
        threads=list(sorted_threads),
        speedup=list(sorted_speedup),
    )


def parse_case_argument(arg: str) -> tuple[str, str, str]:
    try:
        binding, label, path = arg.split(":", 2)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(
            f"Invalid case specification '{arg}'. Expected format binding:label:path"
        ) from exc

    if not binding:
        raise argparse.ArgumentTypeError("Binding must not be empty.")
    if not label:
        raise argparse.ArgumentTypeError("Label must not be empty.")
    if not path:
        raise argparse.ArgumentTypeError("Path must not be empty.")

    return binding, label, path


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Plot parallel scaling behaviour (speedup vs. thread count)."
    )
    parser.add_argument(
        "--case",
        action="append",
        type=parse_case_argument,
        metavar="BINDING:LABEL:PATH",
        dest="cases",
        help="Register a scaling case (e.g. close:\"Small N\" path/to/file.txt). "
        "Provide four entries to capture the requested scenarios.",
    )
    parser.add_argument(
        "--title",
        default="Strong Scaling Performance Benchmark",
        help="Figure title.",
    )
    parser.add_argument(
        "--output",
        help="Optional output path for saving the figure instead of showing it.",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Display the plot even if --output is specified.",
    )
    parser.add_argument(
        "inputs",
        nargs="*",
        help="Optional file paths or glob patterns. If provided (and no --case entries "
        "are given) they will be used to build labels automatically.",
    )
    return parser


def main() -> None:
    parser = build_parser()
    args = parser.parse_args()

    scaling_cases: List[ScalingCase] = []

    def resolve_and_load(binding: str, label: str, path: str) -> None:
        resolved = os.path.abspath(path)
        if not os.path.exists(resolved):
            parser.error(f"File not found: {resolved}")
        try:
            scaling_cases.append(load_scaling_case(binding, label, resolved))
        except RuntimeError as exc:
            print(f"[warn] {exc}")

    if args.cases:
        for binding, label, path in args.cases:
            resolve_and_load(binding, label, path)
    else:
        if not args.inputs:
            parser.error("Provide either --case entries or positional file/glob inputs.")

        matched_files: List[str] = []
        for pattern in args.inputs:
            expanded = glob.glob(pattern)
            if not expanded:
                parser.error(f"No files matched pattern '{pattern}'.")
            matched_files.extend(expanded)

        if not matched_files:
            parser.error("No input files resolved from provided patterns.")

        # ensure stable order
        for path in sorted(set(matched_files)):
            basename = os.path.basename(path)
            binding = "close" if "close" in basename.lower() else (
                "spread" if "spread" in basename.lower() else "unknown"
            )
            label = os.path.splitext(basename)[0]
            resolve_and_load(binding, label, path)

    if not scaling_cases:
        parser.error("No scaling cases were loaded.")

    fig, ax = plt.subplots(figsize=(8, 5))

    scaling_cases.sort(key=lambda case: case.label.lower())

    for scase in scaling_cases:
        label = f"{scase.label} (N={scase.n_value}, {scase.binding})"
        ax.plot(
            scase.threads,
            scase.speedup,
            marker="o",
            label=label,
        )

    ax.set_xscale("log", base=2)
    ax.set_xlabel("Threads")
    ax.set_ylabel("Speedup")
    ax.set_title(args.title)
    ax.grid(alpha=0.6, which="both", linestyle="--")
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))

    fig.tight_layout(rect=[0, 0, 0.78, 1])

    if args.output:
        fig.savefig(args.output, dpi=300)
    if args.show or not args.output:
        plt.show()


if __name__ == "__main__":
    main()

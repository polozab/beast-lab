#!/usr/bin/env python3
"""
Plot helper for Assignment 06 – Task 4 (branch benchmark).

The script consumes the textual output that `branch` prints, e.g.

    Var-Seq  |  size |  total |  perCmd
    ---------+-------+--------+---------
      C-fix1     200   0.434s   1.446ns

and produces the figures requested in Task 4:
  * Variant C per sequence (fix1/fix2/rnd1) for every architecture.
  * Variant D per sequence for every architecture.
  * rnd1 comparison across architectures (variants C and D separately).
  * C vs D comparison (rnd1) per architecture.
  * Optional: Variant E depth sweep (derived from names like E1/E6/E106).

Usage example:

    python plot_branch_task4.py \\
        -i milan=data/milan_C.txt \\
        -i rome=data/rome_C.txt \\
        -o plots/task4

Each input is passed as ARCH=PATH so that the resulting plots can be labelled.
"""

from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional

import matplotlib.pyplot as plt
import pandas as pd


MAX_SIZE = 500_000


RESULT_LINE = re.compile(
    r"^\s*([A-Z][A-Z0-9]*)-([a-z0-9]+)\s+(\d+)\s+([0-9.]+)s\s+([0-9.]+)ns"
)


@dataclass
class InputSpec:
    arch: str
    path: Path


def parse_input_specs(specs: Iterable[str]) -> List[InputSpec]:
    parsed: List[InputSpec] = []
    for spec in specs:
        if "=" not in spec:
            raise ValueError(f"Invalid --input spec '{spec}'. Use ARCH=FILE.")
        arch, file_path = spec.split("=", 1)
        arch = arch.strip()
        if not arch:
            raise ValueError(f"Missing architecture name in spec '{spec}'.")
        path = Path(file_path).expanduser()
        if not path.exists():
            raise FileNotFoundError(path)
        parsed.append(InputSpec(arch=arch, path=path))
    return parsed


def parse_directory_inputs(data_dir: Optional[str]) -> List[InputSpec]:
    if not data_dir:
        return []
    base = Path(data_dir).expanduser()
    if not base.exists():
        raise FileNotFoundError(base)
    specs: List[InputSpec] = []
    for txt in sorted(base.glob("*.txt")):
        specs.append(InputSpec(arch=txt.stem, path=txt))
    return specs


def parse_result_file(spec: InputSpec) -> pd.DataFrame:
    rows = []
    with spec.path.open("r", encoding="utf-8") as fh:
        for line in fh:
            match = RESULT_LINE.match(line)
            if not match:
                continue
            variant = match.group(1)
            sequence = match.group(2)
            rows.append(
                {
                    "arch": spec.arch,
                    "variant": variant,
                    "sequence": sequence,
                    "size": int(match.group(3)),
                    "total_s": float(match.group(4)),
                    "per_cmd_ns": float(match.group(5)),
                }
            )
    return pd.DataFrame(rows)


def ensure_output_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def save_figure(fig: plt.Figure, path: Path) -> None:
    ensure_output_dir(path.parent)
    fig.tight_layout()
    fig.savefig(path, dpi=200)
    plt.close(fig)
    print(f"[+] Wrote {path}")


def plot_variant_sequences(
    df: pd.DataFrame,
    arch: str,
    variant: str,
    sequences: Iterable[str],
    out_dir: Path,
    log_x: bool,
) -> None:
    subset = df[(df["arch"] == arch) & (df["variant"] == variant)]
    if subset.empty:
        return
    fig, ax = plt.subplots(figsize=(6, 4))
    for seq in sequences:
        seq_df = subset[subset["sequence"] == seq].sort_values("size")
        if seq_df.empty:
            continue
        ax.plot(
            seq_df["size"],
            seq_df["per_cmd_ns"],
            marker="o",
            label=f"{variant}-{seq}",
        )
    ax.set_title(f"{arch}")
    ax.set_xlabel("Command sequence size")
    ax.set_ylabel("Per-command time [ns]")
    if log_x:
        ax.set_xscale("log")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(title="Sequence", bbox_to_anchor=(1.02, 1), loc="upper left")
    save_figure(fig, out_dir / f"{arch}_system_variant_{variant}.png")


def plot_sequence_across_arch(
    df: pd.DataFrame,
    variant: str,
    sequence: str,
    out_dir: Path,
    log_x: bool,
) -> None:
    subset = df[(df["variant"] == variant) & (df["sequence"] == sequence)]
    if subset.empty:
        return
    fig, ax = plt.subplots(figsize=(6, 4))
    for arch, arch_df in subset.groupby("arch"):
        arch_df = arch_df.sort_values("size")
        ax.plot(
            arch_df["size"],
            arch_df["per_cmd_ns"],
            marker="o",
            label=arch,
        )
    ax.set_title(f"System comparison for {sequence}")
    ax.set_xlabel("Command sequence size")
    ax.set_ylabel("Per-command time [ns]")
    if log_x:
        ax.set_xscale("log")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(title="Architecture", bbox_to_anchor=(1.02, 1), loc="upper left")
    save_figure(fig, out_dir / f"variant_{variant}_{sequence}_system_comparison.png")


def plot_variant_comparison(
    df: pd.DataFrame,
    arch: str,
    sequence: str,
    variants: Iterable[str],
    out_dir: Path,
    log_x: bool,
) -> None:
    subset = df[(df["arch"] == arch) & (df["sequence"] == sequence)]
    if subset.empty:
        return
    fig, ax = plt.subplots(figsize=(6, 4))
    for variant in variants:
        var_df = subset[subset["variant"] == variant].sort_values("size")
        if var_df.empty:
            continue
        ax.plot(
            var_df["size"],
            var_df["per_cmd_ns"],
            marker="o",
            label=f"Variant {variant}",
        )
    ax.set_title(f"{arch}: sequence {sequence} (variant comparison)")
    ax.set_xlabel("Command sequence size")
    ax.set_ylabel("Per-command time [ns]")
    if log_x:
        ax.set_xscale("log")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(bbox_to_anchor=(1.02, 1), loc="upper left")
    save_figure(fig, out_dir / f"{arch}_{sequence}_variant_compare.png")


def plot_variant_e_depth(
    df: pd.DataFrame,
    arch: str,
    sequence: str,
    out_dir: Path,
) -> None:
    subset = df[(df["arch"] == arch) & (df["sequence"] == sequence)]
    subset = subset[subset["variant"].str.startswith("E")]
    if subset.empty:
        return

    def variant_to_depth(name: str) -> Optional[int]:
        digits = "".join(ch for ch in name[1:] if ch.isdigit())
        return int(digits) if digits else None

    subset = subset.copy()
    subset["depth"] = subset["variant"].map(variant_to_depth)
    subset = subset.dropna(subset=["depth"]).sort_values("depth")
    if subset.empty:
        return

    fig, ax = plt.subplots(figsize=(6, 4))
    ax.plot(
        subset["depth"],
        subset["per_cmd_ns"],
        marker="o",
    )
    ax.set_title(f"{arch}: Variant E depth sweep ({sequence})")
    ax.set_xlabel("E-depth (recursive batch length)")
    ax.set_ylabel("Per-command time [ns]")
    ax.grid(True, alpha=0.3)
    save_figure(fig, out_dir / f"{arch}_variant_E_depth_{sequence}.png")


def plot_rnd1_all_architectures(
    df: pd.DataFrame,
    variant: str,
    out_dir: Path,
    log_x: bool,
) -> None:
    subset = df[(df["sequence"] == "rnd1") & (df["variant"] == variant)]
    if subset.empty:
        return

    fig, ax = plt.subplots(figsize=(7, 4))
    arches = sorted(subset["arch"].unique())
    colors = plt.cm.get_cmap("tab10", len(arches))
    arch_color: Dict[str, tuple] = {arch: colors(i) for i, arch in enumerate(arches)}

    for arch, group in subset.groupby("arch"):
        group = group.sort_values("size")
        ax.plot(
            group["size"],
            group["per_cmd_ns"],
            label=arch,
            color=arch_color[arch],
            linestyle="-",
            marker="o",
        )

    ax.set_title("rnd1 across all systems")
    ax.set_xlabel("Command sequence size")
    ax.set_ylabel("Per-command time [ns]")
    if log_x:
        ax.set_xscale("log")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(title="Systems", bbox_to_anchor=(1.02, 1), loc="upper left")
    save_figure(fig, out_dir / "rnd1_across_systems.png")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate Task 4 branch benchmark plots."
    )
    parser.add_argument(
        "-i",
        "--input",
        action="append",
        required=False,
        help="Input spec ARCH=RESULT_FILE (can be repeated).",
    )
    parser.add_argument(
        "-d",
        "--data-dir",
        action="append",
        help="Directory containing ARCH.txt files (auto-detected). Can be repeated.",
    )
    parser.add_argument(
        "-o",
        "--output",
        default="plots",
        help="Directory to store generated figures (default: ./plots).",
    )
    parser.add_argument(
        "--logx",
        action="store_true",
        help="Use logarithmic scaling on the x-axis.",
    )
    parser.add_argument(
        "--variant",
        default="C",
        help="Branch variant to plot (default: C).",
    )
    args = parser.parse_args()

    specs = []
    if args.data_dir:
        for data_dir in args.data_dir:
            specs.extend(parse_directory_inputs(data_dir))
    if args.input:
        specs.extend(parse_input_specs(args.input))
    if not specs:
        parser.error("No inputs provided. Use --data-dir or --input.")

    frames = [parse_result_file(spec) for spec in specs]
    df = pd.concat(frames, ignore_index=True)
    df = df[df["size"] <= MAX_SIZE].copy()

    out_dir = Path(args.output)
    out_dir = out_dir / "newPlot"
    ensure_output_dir(out_dir)

    variant = args.variant.upper()
    seqs = ["fix1", "fix2", "rnd1"]
    for arch in sorted(df["arch"].unique()):
        plot_variant_sequences(df, arch, variant, seqs, out_dir, args.logx)
        plot_variant_comparison(df, arch, "rnd1", ["C", "D"], out_dir, args.logx)

    plot_rnd1_all_architectures(df, variant, out_dir, args.logx)


if __name__ == "__main__":
    main()

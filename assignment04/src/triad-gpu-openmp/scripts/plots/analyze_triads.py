#!/usr/bin/env python3
"""
Aggregate MFLOPS results from multiple triad benchmark CSV files and report the
strongest (teams, threads) configurations.

Example:
    python analyze_triads.py results/*.csv --top 10 --summary-csv summary.csv
"""

from __future__ import annotations

import argparse
import csv
import statistics
from collections import defaultdict
from pathlib import Path
from typing import Dict, Iterable, List, Tuple


Record = Dict[str, float]
ConfigKey = Tuple[int, int]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Analyze triad MFLOPS results.")
    parser.add_argument("inputs", nargs="+", help="Input CSV files (wildcards allowed).")
    parser.add_argument(
        "--top",
        type=int,
        default=5,
        help="How many (teams, threads) configurations to print (default: 5).",
    )
    parser.add_argument(
        "--summary-csv",
        type=Path,
        help="Optional file to write the aggregated summary table to.",
    )
    parser.add_argument(
        "--sort-by",
        choices=("max", "avg", "median", "combined"),
        default="max",
        help="Metric used to rank configurations (default: max).",
    )
    return parser.parse_args()


def load_records(path: Path) -> List[Record]:
    records: List[Record] = []
    with path.open(newline="") as handle:
        reader = csv.DictReader(handle, skipinitialspace=True)
        for row in reader:
            try:
                record: Record = {
                    "file": str(path),
                    "data_size": int(row["Data size"]),
                    "teams": int(row["Teams"]),
                    "threads": int(row["Threads"]),
                    "mflops": float(row["MFLOPS"]),
                    "cycles": int(row["Cycles"]),
                    "gpu": int(row["GPU"]),
                    "checksum": float(row["Checksum"]),
                }
            except (KeyError, ValueError) as exc:
                raise ValueError(f"Malformed row in {path}: {row}") from exc
            records.append(record)
    return records


def aggregate(records: Iterable[Record]) -> Dict[ConfigKey, Dict[str, float]]:
    stats: Dict[ConfigKey, Dict[str, float]] = defaultdict(
        lambda: {
            "total_mflops": 0.0,
            "values": [],
            "max_mflops": float("-inf"),
            "best_record": None,
            "count": 0,
        }
    )
    for record in records:
        key = (record["teams"], record["threads"])
        entry = stats[key]
        entry["total_mflops"] += record["mflops"]
        entry["values"].append(record["mflops"])
        entry["count"] += 1
        if record["mflops"] > entry["max_mflops"]:
            entry["max_mflops"] = record["mflops"]
            entry["best_record"] = record
    for entry in stats.values():
        entry["avg_mflops"] = entry["total_mflops"] / entry["count"]
        entry["median_mflops"] = statistics.median(entry["values"])
    return stats


def metric_key(entry: Dict[str, float], sort_by: str) -> float:
    if sort_by == "avg":
        return entry["avg_mflops"]
    if sort_by == "median":
        return entry["median_mflops"]
    return entry["max_mflops"]


def metric_ranks(stats: Dict[ConfigKey, Dict[str, float]], metric: str) -> Dict[ConfigKey, int]:
    ordered = sorted(
        stats.items(),
        key=lambda kv: metric_key(kv[1], metric),
        reverse=True,
    )
    return {key: rank for rank, (key, _) in enumerate(ordered, start=1)}


def combined_ranks(stats: Dict[ConfigKey, Dict[str, float]]) -> Dict[ConfigKey, int]:
    ranking = {
        metric: metric_ranks(stats, metric)
        for metric in ("max", "avg", "median")
    }
    combined: Dict[ConfigKey, int] = {}
    for key in stats:
        combined[key] = sum(ranking[metric][key] for metric in ranking)
    return combined


def print_summary(
    stats: Dict[ConfigKey, Dict[str, float]],
    top: int,
    sort_by: str,
    combined_rankings: Dict[ConfigKey, int] | None = None,
) -> None:
    if not stats:
        print("No records to summarize.")
        return
    if sort_by == "combined":
        if combined_rankings is None:
            combined_rankings = combined_ranks(stats)
        sorted_items = sorted(
            stats.items(),
            key=lambda kv: (
                combined_rankings[kv[0]],
                -kv[1]["max_mflops"],
            ),
        )
    else:
        sorted_items = sorted(
            stats.items(),
            key=lambda kv: (
                metric_key(kv[1], sort_by),
                kv[1]["avg_mflops"],
            ),
            reverse=True,
        )
    print(
        f"\nTop {min(top, len(sorted_items))} configurations "
        f"(sorted by {sort_by.upper()} MFLOPS):"
    )
    header = (
        "Rank",
        "Teams",
        "Threads",
        "Max MFLOPS",
        "Avg MFLOPS",
        "Median MFLOPS",
        "Best data size",
        "Cycles @ best",
        "Source file",
    )
    print("{:>4} {:>8} {:>8} {:>12} {:>12} {:>14} {:>14} {:>12} {}".format(*header))
    for rank, (key, entry) in enumerate(sorted_items[:top], start=1):
        best = entry["best_record"]
        print(
            f"{rank:>4} {key[0]:>8} {key[1]:>8} "
            f"{entry['max_mflops']:>12.2f} {entry['avg_mflops']:>12.2f} "
            f"{entry['median_mflops']:>14.2f} {best['data_size']:>14} "
            f"{best['cycles']:>12} {best['file']}"
        )


def write_summary_csv(
    stats: Dict[ConfigKey, Dict[str, float]],
    path: Path,
    sort_by: str,
    combined_rankings: Dict[ConfigKey, int] | None = None,
) -> None:
    fieldnames = [
        "teams",
        "threads",
        "count",
        "max_mflops",
        "avg_mflops",
        "median_mflops",
        "best_data_size",
        "best_cycles",
        "combined_rank",
        "source_file",
    ]
    with path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        if sort_by == "combined" and combined_rankings:
            ordering = sorted(
                stats.items(),
                key=lambda kv: (
                    combined_rankings[kv[0]],
                    -kv[1]["max_mflops"],
                ),
            )
        else:
            ordering = sorted(
                stats.items(),
                key=lambda kv: (
                    metric_key(kv[1], sort_by),
                    kv[1]["avg_mflops"],
                ),
                reverse=True,
            )
        for (teams, threads), entry in ordering:
            best = entry["best_record"]
            writer.writerow(
                {
                    "teams": teams,
                    "threads": threads,
                    "count": entry["count"],
                    "max_mflops": entry["max_mflops"],
                    "avg_mflops": entry["avg_mflops"],
                    "median_mflops": entry["median_mflops"],
                    "best_data_size": best["data_size"],
                    "best_cycles": best["cycles"],
                    "combined_rank": combined_rankings[(
                        teams,
                        threads,
                    )]
                    if combined_rankings
                    else "",
                    "source_file": best["file"],
                }
            )
    print(f"Wrote summary CSV to {path}")


def main() -> None:
    args = parse_args()
    files: List[Path] = []
    for pattern in args.inputs:
        matched = list(Path().glob(pattern))
        if matched:
            files.extend(matched)
        else:
            files.append(Path(pattern))
    all_records: List[Record] = []
    for path in files:
        if not path.exists():
            raise FileNotFoundError(path)
        records = load_records(path)
        print(f"Loaded {len(records):>4} rows from {path}")
        all_records.extend(records)
    stats = aggregate(all_records)
    combined_rankings = combined_ranks(stats) if stats and args.sort_by == "combined" else None
    print_summary(stats, args.top, args.sort_by, combined_rankings)
    if args.summary_csv:
        write_summary_csv(stats, args.summary_csv, args.sort_by, combined_rankings)


if __name__ == "__main__":
    main()

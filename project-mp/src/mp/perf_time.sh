#!/usr/bin/env bash
set -euo pipefail

N=100

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <command...>"
  echo "Example:"
  echo "  $0 bin/mp input/randomlist128.txt 256 10"
  exit 1
fi

CMD=("$@")

echo "Command:"
echo "  perf stat ${CMD[*]}"
echo "Runs: $N"
echo

# Collect times (one per line)
times=$(mktemp)
trap 'rm -f "$times"' EXIT

for i in $(seq 1 "$N"); do
  perf stat "${CMD[@]}" 2>&1 \
    | awk '/seconds time elapsed/ {print $1}' >> "$times"
done

# Compute min / avg / max in awk
awk '
NR == 1 {
  min = max = sum = $1
  next
}
{
  if ($1 < min) min = $1
  if ($1 > max) max = $1
  sum += $1
}
END {
  avg = sum / NR
  printf "Results over %d runs:\n", NR
  printf "  min: %.9f s\n", min
  printf "  avg: %.9f s\n", avg
  printf "  max: %.9f s\n", max
}
' "$times"

from pathlib import Path
from plotnine import *
import pandas as pd
import re


def parse_clock_frequencies(txt):
  
    target_regions = ["float", "double", "int32_t", "int64_t"]
    region_re = re.compile(r"Region\s+([A-Za-z0-9_]+),")
    clock_re = re.compile(r"Clock\s*\[MHz\]\s*\|\s*([0-9.]+)")

    result = {}
    current_region = None

    for line in txt.splitlines():
        m_region = region_re.search(line)
        if m_region:
            name = m_region.group(1)
            if name in target_regions:
                current_region = name
            continue

        m_clock = clock_re.search(line)
        if m_clock and current_region is not None:
            MHz = float(m_clock.group(1))
            result[current_region] = MHz
            current_region = None

    return result

max_chains = "50"

base = Path("./data")

dfs = []

for f in base.iterdir():
  
  if max_chains not in f.stem:
    continue
  
  if f.suffix == ".csv":
    
    df = pd.read_csv(f, skipinitialspace=True)
    df["system"] = f.stem.split("_")[0]
    df["clock_frq"] = df["dtype"]
    
    txt_file = base / (f.stem + ".txt")
    txt_str = txt_file.read_text()
    freq_map = parse_clock_frequencies(txt_str)
    df["clock_frq"] = df["clock_frq"].map(freq_map)
    
    dfs.append(df)

dfs = pd.concat(dfs, ignore_index=True)

seq_dfs = dfs[dfs["chains"] == 1][["dtype", "op", "time", "system", "clock_frq"]]
seq_dfs["latency"] = seq_dfs["time"]
print(seq_dfs)



dfs = dfs.drop(columns=["rep", "clock_frq"])
best = dfs.loc[dfs.groupby(["op", "dtype", "system"])["time"].idxmin()]

plot = ggplot(best, aes(x="op", y="time", color="dtype")) + scale_y_log10() + geom_point() + facet_wrap("system")
plot.save("./plots/time_comp.png")

plot = ggplot(best, aes(x="op", y="chains", color="dtype")) + geom_point() + facet_wrap("system")
plot.save("./plots/chains_comp.png")
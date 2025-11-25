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

dfs = dfs[dfs["chains"] == 1]
dfs["latency"] = dfs["time"] * dfs["clock_frq"] * 10 ** 6 / dfs["rep"]
dfs["latency_label"] = round(dfs["latency"], 2)

dfs = dfs.drop(columns=["rep", "clock_frq", "chains"])

plot = ggplot(dfs, aes(x="op", y="latency", label="latency_label", fill="dtype")) + geom_bar(stat="identity") + facet_grid("~system")
plot.save("./plots/latency_overview.svg", height=6, width=12)

plot = ggplot(dfs[dfs["op"] != "/"], aes(x="op", y="latency", label="latency_label", fill="dtype")) + geom_bar(stat="identity") + facet_grid("~system")
plot.save("./plots/latency_nodiv.svg", height=6, width=12)

plot = ggplot(dfs[dfs["system"] != "cs2"], aes(x="op", y="latency", label="latency_label", fill="dtype")) + geom_bar(stat="identity") + facet_grid("~system")
plot.save("./plots/latency_nocs2.svg", height=6, width=12)

plot = ggplot(dfs[(dfs["system"] != "cs2") & (dfs["op"] != "/")], aes(x="op", y="latency", label="latency_label", fill="dtype")) + geom_bar(stat="identity") + facet_grid("~system")
plot.save("./plots/latency_nocs2_nodiv.svg", height=6, width=12)

plot = ggplot(dfs[dfs["system"] == "cs2"], aes(x="dtype", y="latency", label="latency_label")) + geom_bar(stat="identity") + geom_label() + facet_wrap("op", scales="free_y") + labs(title="cs2")
plot.save("./plots/latency_overview_cs2.svg", height=6, width=12)

plot = ggplot(dfs[dfs["system"] == "sap3"], aes(x="dtype", y="latency", label="latency_label")) + geom_bar(stat="identity") + geom_label() + facet_wrap("op", scales="free_y") + labs(title="sap3")
plot.save("./plots/latency_overview_sap3.svg", height=6, width=12)

plot = ggplot(dfs[dfs["system"] == "milan2"], aes(x="dtype", y="latency", label="latency_label")) + geom_bar(stat="identity") + geom_label() + facet_wrap("op", scales="free_y") + labs(title="milan2")
plot.save("./plots/latency_overview_milan2.svg", height=6, width=12)

plot = ggplot(dfs[dfs["system"] == "gh3"], aes(x="dtype", y="latency", label="latency_label")) + geom_bar(stat="identity") + geom_label() + facet_wrap("op", scales="free_y") + labs(title="gh3")
plot.save("./plots/latency_overview_gh3.svg", height=6, width=12)



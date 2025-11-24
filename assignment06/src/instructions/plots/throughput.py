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

best = dfs.loc[dfs.groupby(["op", "dtype", "system"])["time"].idxmin()]

plot = ggplot(best, aes(x="op", y="chains", color="dtype", label="chains")) + geom_label() + facet_grid("~system")
plot.save("./plots/chains_best.svg", height=6, width=12)

best["IPC"] = best["rep"] / (best["time"] * best["clock_frq"] * 1.e+6)
best["IPC_label"] = round(best["IPC"], 2)
best["CPI"] = 1 / best["IPC"]
best["CPI_label"] = round(best["CPI"], 2)

best = best.drop(columns=["rep", "clock_frq", "chains"])

#
# ICP
#

plot = ggplot(best[best["system"] == "gh3"], aes(x="dtype", y="IPC", label="IPC_label")) + geom_bar(stat="identity") + geom_label() +facet_wrap("op", scales="free_y") + labs(title="gh3")
plot.save("./plots/IPC_overview_gh3.svg", height=6, width=12)

plot = ggplot(best[best["system"] == "cs2"], aes(x="dtype", y="IPC", label="IPC_label")) + geom_bar(stat="identity") + geom_label() +facet_wrap("op", scales="free_y") + labs(title="cs2")
plot.save("./plots/IPC_overview_cs2.svg", height=6, width=12)

plot = ggplot(best[best["system"] == "sap3"], aes(x="dtype", y="IPC", label="IPC_label")) + geom_bar(stat="identity") + geom_label() +facet_wrap("op", scales="free_y") + labs(title="sap3")
plot.save("./plots/IPC_overview_sap3.svg", height=6, width=12)

plot = ggplot(best[best["system"] == "milan2"], aes(x="dtype", y="IPC", label="IPC_label")) + geom_bar(stat="identity") + geom_label() +facet_wrap("op", scales="free_y") + labs(title="milan2")
plot.save("./plots/IPC_overview_milan2.svg", height=6, width=12)

#
# CPI
#

plot = ggplot(best[best["system"] == "gh3"], aes(x="dtype", y="CPI", label="CPI_label")) + geom_bar(stat="identity") + geom_label() +facet_wrap("op", scales="free_y") + labs(title="gh3")
plot.save("./plots/CPI_overview_gh3.svg", height=6, width=12)

plot = ggplot(best[best["system"] == "cs2"], aes(x="dtype", y="CPI", label="CPI_label")) + geom_bar(stat="identity") + geom_label() +facet_wrap("op", scales="free_y") + labs(title="cs2")
plot.save("./plots/CPI_overview_cs2.svg", height=6, width=12)

plot = ggplot(best[best["system"] == "sap3"], aes(x="dtype", y="CPI", label="CPI_label")) + geom_bar(stat="identity") + geom_label() +facet_wrap("op", scales="free_y") + labs(title="sap3")
plot.save("./plots/CPI_overview_sap3.svg", height=6, width=12)

plot = ggplot(best[best["system"] == "milan2"], aes(x="dtype", y="CPI", label="CPI_label")) + geom_bar(stat="identity") + geom_label() +facet_wrap("op", scales="free_y") + labs(title="milan2")
plot.save("./plots/CPI_overview_milan2.svg", height=6, width=12)



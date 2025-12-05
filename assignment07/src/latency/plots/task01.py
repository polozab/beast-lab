from pathlib import Path
import pandas as pd
import plotnine as p9

base = Path("./data")
dfs = []

for f in base.iterdir():
  df = pd.read_csv(f, skipinitialspace=True)
  df["system"] = f.stem
  df["measure"] = df["Fun"].astype(str) + " k=" + df["k"].astype(str)
  dfs.append(df)
  
dfs = pd.concat(dfs, ignore_index=True)

print(dfs)

plot = (
    p9.ggplot(dfs, p9.aes(x="N", y="Latency[ns]", color="measure"))
  + p9.scale_x_log10()
  + p9.geom_line()
  + p9.geom_point()
  #+ p9.geom_vline(p9.aes(xintercept=10**8), color="red")
  + p9.facet_wrap("system", scales="free")
)
plot.save("./plots/task1_all.svg", width=12., height=8.)

plot = (
    p9.ggplot(dfs[dfs["system"] == "sap2"], p9.aes(x="N", y="Latency[ns]", color="measure"))
  + p9.scale_x_log10()
  + p9.geom_line()
  + p9.geom_point()
)
plot.save("./plots/task1_sap2.svg", width=12., height=8.)

plot = (
    p9.ggplot(dfs[(dfs["system"] == "sap2") & (dfs["measure"] != "sum_indexload k=gold")], p9.aes(x="N", y="Latency[ns]", color="measure"))
  + p9.scale_x_log10()
  + p9.geom_line()
  + p9.geom_point()
)
plot.save("./plots/task1_sap2_no-gold-load.svg", width=12., height=8.)

plot = (
    p9.ggplot(dfs[(dfs["system"] == "sap2") & (dfs["k"] != "gold")], p9.aes(x="N", y="Latency[ns]", color="measure"))
  + p9.scale_x_log10()
  + p9.geom_line()
  + p9.geom_point()
)
plot.save("./plots/task1_sap2_no-gold.svg", width=12., height=8.)

plot = (
    p9.ggplot(dfs[dfs["system"] == "milan1"], p9.aes(x="N", y="Latency[ns]", color="measure"))
  + p9.scale_x_log10()
  + p9.geom_line()
  + p9.geom_point()
  #+ p9.geom_vline(p9.aes(xintercept=10**8), color="red")
  + p9.facet_grid("k", scales="free")
)
plot.save("./plots/task1_milan1.svg", width=12., height=8.)

plot = (
    p9.ggplot(dfs[dfs["system"] == "gh3"], p9.aes(x="N", y="Latency[ns]", color="measure"))
  + p9.scale_x_log10()
  + p9.geom_line()
  + p9.geom_point()
  #+ p9.geom_vline(p9.aes(xintercept=10**8), color="red")
  + p9.facet_grid("k", scales="free")
)
plot.save("./plots/task1_gh3.svg", width=12., height=8.)

plot = (
    p9.ggplot(dfs[dfs["system"] == "cs7"], p9.aes(x="N", y="Latency[ns]", color="measure"))
  + p9.scale_x_log10()
  + p9.geom_line()
  + p9.geom_point()
  #+ p9.geom_vline(p9.aes(xintercept=10**8), color="red")
  + p9.facet_grid("k", scales="free")
)
plot.save("./plots/task1_cs7.svg", width=12., height=8.)

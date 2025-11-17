from glob import glob
from pathlib import Path
import pandas as pd
from plotnine import *


base = Path("./data/t2")
dfs = []


for f in base.iterdir():
  if "tiling" in f.stem:
    try:
      df = pd.read_csv(f, skipinitialspace=True)
      df["Mem_Order"] = "col" if "col" in f.stem else "row"
      dfs.append(df)
    except Exception as _:
      pass
                        

dfs = pd.concat(dfs)[["Tiles", "Mem_Order", "MFLOPS"]]
dfs = dfs.groupby(["Tiles", "Mem_Order"]).agg(["max", "mean"]).reset_index()

dfs.columns = ['_'.join([str(part) for part in col if part not in (None, '')]).rstrip('_') for col in dfs.columns.values]
dfs = dfs.melt(["Tiles", "Mem_Order"])

print(dfs)

plot = (ggplot(dfs, aes("Tiles", "value", color="Mem_Order", linetype="variable"))
        + geom_line()
        + scale_x_continuous(name="Tile Size")
        + scale_y_continuous(name="MFLOPS/s")
        + scale_color_discrete(
                name="Memory Order",
                labels={"col": "Column-Major", "row": "Row-Major"}
        )
        + scale_linetype_discrete(
                name="Metric",
                labels={"MFLOPS_max": "Max", "MFLOPS_mean": "Average"}
        )
        )
plot.save("./data/t2_tiling.png")

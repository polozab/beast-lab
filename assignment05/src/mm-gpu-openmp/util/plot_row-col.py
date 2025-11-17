from glob import glob
from pathlib import Path
import pandas as pd
from plotnine import *

base = Path("./data/t2")
dfs = []

for f in base.iterdir():
  if f.stem in ("explicit_col", "explicit_row", "implicit_col", "implicit_row"):
    try:
      df = pd.read_csv(f, skipinitialspace=True)
      df["Mem_Order"] = "col" if "col" in f.stem else "row"
      if "explicit" in f.stem:
        df["Measure"] = "explicit"
      elif "implicit" in f.stem:
        df["Measure"] = "implicit"
      dfs.append(df)
    except Exception as _:
      pass
          
dfs = pd.concat(dfs)[["Mem_Order", "Measure", "MFLOPS", "N"]]

plot = (ggplot(dfs, aes(x="N", y="MFLOPS", color="Measure", linetype="Mem_Order"))
        + geom_line()
        + scale_x_log10(name="Matrix Size")
        + scale_y_continuous(name="MFLOPS/s")
        + scale_color_discrete(
          name="Measurement",
          labels={"explicit": "Explicit pragma", "implicit": "Implicit pragma", "tiling": "Explicit pragma with tiling"}
        )
        + scale_linetype_discrete(
          name="Memory Order",
          labels={"col": "Column-Major", "row": "Row-Major"}
        )
        )
plot.save("./data/t2_row-col.png")
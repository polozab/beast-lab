from glob import glob
from pathlib import Path
import pandas as pd
from plotnine import *

base = Path("./data/t2")
dfs = []

for f in base.iterdir():
  if f.stem in ("explicit_row", "implicit_row"):
    try:
      df = pd.read_csv(f, skipinitialspace=True)
      df["Measure"] = "explicit" if "explicit" in f.stem else "implicit"
      dfs.append(df)
    except Exception as _:
      pass
          
dfs = pd.concat(dfs)[["Measure", "MFLOPS", "N"]]

plot = (ggplot(dfs, aes(x="N", y="MFLOPS", color="Measure"))
        + geom_line()
        + scale_x_log10(name="Matrix Size")
        + scale_y_continuous(name="MFLOPS/s")
        + scale_color_discrete(
          name="Measurement",
          labels={"explicit": "Explicit pragma", "implicit": "Implicit pragma", "tiling": "Explicit pragma with tiling"}
        )
        )
plot.save("./data/t2_expl-impl.png")
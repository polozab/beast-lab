from pathlib import Path
import pandas as pd
from plotnine import ggplot, aes, geom_line, scale_x_log10, scale_y_continuous, scale_color_discrete

base = Path("./assignment04")

dfs = []

for f in (base / "src/triad-gpu-openmp/data/memory-init").iterdir():
  df = pd.read_csv(f, skipinitialspace=True)
  df["mem_init"] = f.stem
  dfs.append(df)

df = pd.concat(dfs)

df = df[df["mem_init"].isin(["cpu", "gpu"])]

print(df.columns)

p = (ggplot(df, aes(x="Data size", y="MFLOPS", color="mem_init"))
     
     + scale_x_log10(
       name="Vector Size (Elements)"
     )
     + scale_y_continuous(
       name="MFLOPS/s"
     )
     + scale_color_discrete(
        name="Memory Initialization",
        breaks=["cpu", "gpu"],
        labels=["On CPU", "On GPU"]
     )
     + geom_line())

p.save((base / "report/task1-plots/memory-init.png"))
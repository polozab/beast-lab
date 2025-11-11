from pathlib import Path
import pandas as pd
from plotnine import ggplot, aes, geom_line, scale_x_log10, scale_y_continuous, scale_color_discrete

base = Path("./assignment04")

dfs = []

for f in (base / "src/triad-gpu-openmp/data").iterdir():
  df = pd.read_csv(f, skipinitialspace=True)
  df["scheduling"] = f.stem
  dfs.append(df)

df = pd.concat(dfs)

df = df[df["scheduling"].isin(["scheduling_none", "scheduling_static-1"])]

print(df.columns)

p = (ggplot(df, aes(x="Data size", y="MFLOPS", color="scheduling"))
     
     + scale_x_log10(
       name="Vector Size (Elements)"
     )
     + scale_y_continuous(
       name="MFLOPS/s"
     )
     + scale_color_discrete(
        name="Thread Scheduling",
        breaks=["scheduling_none", "scheduling_static-1"],
        labels=["None defined", "Static scheduling"]
    )
     + geom_line())

p.save((base / "report/task1-plots/scheduling.png"))
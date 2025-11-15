import pandas as pd
from plotnine import *


df = pd.read_csv("./data/result.csv", skipinitialspace=True)
print(df.columns)

df["Teams x Threads"] = f"{df.iloc[0,2]} x {df.iloc[0,3]}"

print(df.head())

plot = (ggplot(df, aes(x="N", y="MFLOPS", color="Teams x Threads"))
        + geom_vline(xintercept=300)
        + lims(y=(0, df["MFLOPS"].max()))
        + geom_line()
        )

plot.save("./data/plot.png")
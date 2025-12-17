import pandas as pd
from glob import glob


files = glob("./spread_*")

dfs: list[pd.DataFrame] = []

for f in files:
  df = pd.read_csv(f, skipinitialspace=True)
  dfs.append(df)
  
dfss = pd.concat(dfs)

dfss = dfss.groupby("N")["MFLOPS"].max().reset_index()
print(dfss)
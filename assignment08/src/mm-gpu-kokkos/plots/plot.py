import pandas as pd
import plotnine as p9

df = pd.read_csv("./data/kokkos_kernel_mm.csv", skipinitialspace=True)
df = df.melt(["size"], var_name="measure", value_name="value")

df["device"] = df["measure"].str.startswith("cpu_").map({True: "CPU", False: "GPU"})
df["measure"] = df["measure"].str.replace(r"^(cpu|gpu)_", "", regex=True)

print(df)

df_filtered = df[df["measure"] == "avg_gflops"]
plot = p9.ggplot(df_filtered, p9.aes(x="size", y="value", color="device")) + p9.geom_line() + p9.geom_point() + p9.labs(y="GFLOPS/s", x="Matrix Size", color="Device")
plot.save("./plots/kokkos_kernel_gflops.svg")

df_filtered = df[df["measure"] == "avg_time_s"]
plot = p9.ggplot(df_filtered, p9.aes(x="size", y="value", color="device")) + p9.geom_line() + p9.geom_point() + p9.labs(y="Execution Time (s)", x="Matrix Size", color="Device")
plot.save("./plots/kokkos_kernel_time.svg")
import pandas as pd
import plotnine as p9

df = pd.read_csv("./data/kokkos_kernel_mm_comp.csv", skipinitialspace=True)
df = df.melt(["size"], var_name="measure", value_name="value")

df["kernel"] = df["measure"].str.startswith("kernel_").map({True: "Kokkos", False: "Handwritten"})
df["measure"] = df["measure"].str.replace(r"^(kernel|handwritten)_", "", regex=True)

print(df)

df_filtered = df[df["measure"] == "avg_gflops"]
plot = p9.ggplot(df_filtered, p9.aes(x="size", y="value", color="kernel")) + p9.geom_line() + p9.geom_point() + p9.labs(y="GFLOPS/s", x="Matrix Size", color="Version")
plot.save("./plots/kernel_comp_flops.svg")

df_filtered = df[df["measure"] == "avg_time_s"]
plot = p9.ggplot(df_filtered, p9.aes(x="size", y="value", color="kernel")) + p9.geom_line() + p9.geom_point() + p9.labs(y="Execution Time (s)", x="Matrix Size", color="Version")
plot.save("./plots/kernel_comp_time.svg")
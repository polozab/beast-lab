import pandas as pd
import plotnine as p9
from glob import glob

omp_gpu = pd.read_csv("./data/omp_gpu.csv", skipinitialspace=True)
omp_gpu["size"] = omp_gpu["N"]
omp_gpu["value"] = omp_gpu["MFLOPS"] / 1000
omp_gpu["kernel"] = "OMP GPU"
omp_gpu["measure"] = "avg_gflops"

omp_gpu = omp_gpu[["size", "kernel", "measure", "value"]]

omp_cpu = pd.read_csv("./data/omp_cpu.csv", skipinitialspace=True, delim_whitespace=True)
omp_cpu["size"] = omp_cpu["N"]
omp_cpu["value"] = omp_cpu["MFLOPS"] / 1000
omp_cpu["kernel"] = "OMP CPU"
omp_cpu["measure"] = "avg_gflops"

omp_cpu = omp_cpu[["size", "kernel", "measure", "value"]]


df = pd.read_csv("./data/kokkos_kernel_mm_comp.csv", skipinitialspace=True)
df = df.melt(["size"], var_name="measure", value_name="value")

df["kernel"] = df["measure"].str.startswith("kernel_").map({True: "Kokkos", False: "Handwritten"})
df["measure"] = df["measure"].str.replace(r"^(kernel|handwritten)_", "", regex=True)
df = df[df["kernel"] == "Handwritten"]
df["kernel"] = "Kokkos GPU"

df = pd.concat([df, omp_gpu, omp_cpu])

print(df)

df_filtered = df[df["measure"] == "avg_gflops"]
plot = p9.ggplot(df_filtered, p9.aes(x="size", y="value", color="kernel")) + p9.geom_line() + p9.geom_point() + p9.labs(y="GFLOPS/s", x="Matrix Size", color="Version")
plot.save("./plots/kokkos_omp.svg")
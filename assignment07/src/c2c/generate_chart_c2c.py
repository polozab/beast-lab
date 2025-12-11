from matplotlib import pyplot as plt
import matplotlib as mpl
import seaborn as sns
import pandas as pd
import numpy as np
import sys

# check num arguments
if len(sys.argv) < 2:
    print('Usage: python plot_heatmap.py <file>')
    exit(1)

# read input file
filename = sys.argv[1]
df = pd.read_csv(filename, header=None)

# add columns and indices
N = df.shape[0]
columns = []
indices = []
for i in range(N):
    label = 'C' + str(i)
    columns.append(label)
    indices.append(label)
df.columns = columns
df.index = indices

# check the matrix
print(df)

# for masking null values
cmap = mpl.colormaps.get_cmap('Greens')
cmap.set_bad("black")

# plot the heatmap
plt.figure()
hm = sns.heatmap(df, cmap=cmap, linewidths=0.30, annot=False,  fmt=".1f", mask=(df==0.0))
# remove mask=(df==0.0) if not necessary

#output = './heatmap_avg_c2clatency_sap.pdf'
#output = './heatmap_avg_c2clatency_milan.pdf'
#output = './heatmap_avg_c2clatency_gh.pdf'
output = './heatmap_avg_c2clatency_cs.pdf'
#output = './heatmap_avg_c2clatency_fujitsu.pdf'
# output = './heatmap_avg_c2clatency_custom.pdf'

# save to file
print('--------------------------------------------')
print('Write to file: {}'.format(output))
#plt.title('fujitsu_48_cores', size=14)
#plt.title('sap_96_cores', size=14)        # 64 sockets, 1 core each
#plt.title('milan_128_cores', size=14)     # 2 sockets, 64 cores each
#plt.title('gh_72_cores', size=14)         # 1 socket, 72 cores
plt.title('cs_48_cores', size=14)         # 4 sockets, 12 cores each
# plt.title('fujitsu_48_cores', size=14)    # A64FX, 4 NUMA, 48 cores

plt.savefig(output, bbox_inches='tight')
print('--------------------------------------------')

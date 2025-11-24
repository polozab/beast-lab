# Report on Task 4

## a) Comparing indirect jumps on different architectures and sizes

### Measurement on Milan
![](newPlot/milan_system_variant_C.png)
>**Note**: The Measurement of the C-fix1 and C-fix2 are nearly identical.
### Measurement on Sap
![](newPlot/sap_system_variant_C.png)
### Measurement on GH
![](newPlot/gh_system_variant_C.png)
### Measurement on CS
![](newPlot/cs_system_variant_C.png)

---

### Measurement on all architectures for rnd1
![](newPlot/rnd1_across_systems.png)
### Analysis

The two `fix` meassurements show almost no difference on the command sequence size and are more or less constant over the whole size range. This indicates that the CPU is able to effectively predict the indirect jumps even for small command sequences.

The `rnd1`measurements show also that all architectures reach a plateau. all systems appart from gh start with a low per command execution time and then jump rather rapidly to a higher execution time where they than stay almost constant. The gh on the other side aproaches its plateau more slowly and steadily.

## b) Comparing conditional jumps on different architectures and sizes
### Measurement on Milan
![](newPlot/b/milan_system_variant_D.png)
### Measurement on Sap
![](newPlot/b/sap_system_variant_D.png)
### Measurement on GH
![](newPlot/b/gh_system_variant_D.png)
### Measurement on CS
![](newPlot/b/cs_system_variant_D.png)

---

### Measurement on all architectures for rnd1
![](newPlot/b/rnd1_across_systems.png)

---

### Comparison of variant C and D on Milan
![](newPlot/b/compare/milan_rnd1_variant_compare.png)
### Comparison of variant C and D on Sap
![](newPlot/b/compare/sap_rnd1_variant_compare.png)
### Comparison of variant C and D on GH
![](newPlot/b/compare/gh_rnd1_variant_compare.png)
### Comparison of variant C and D on CS
![](newPlot/b/compare/cs_rnd1_variant_compare.png)

### Analysis

### Appendix
- Measurement data can be found [here](/Group-101/assignment06/src/plots/data/)
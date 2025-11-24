# Report on Task 3

## a) Testing for indirect jumps

### Measurement on Milan
| Var-Seq | size | total  | perCmd   |
|---------|------|--------|----------|
| A-fix1  | 200  | 0.701s | 2.336ns  |
| A-fix2  | 200  | 0.701s | 2.336ns  |
| B-fix1  | 200  | 0.537s | 1.790ns  |
| B-fix2  | 200  | 0.537s | 1.791ns  |
| C-fix1  | 200  | 0.435s | 1.450ns  |
| C-fix2  | 200  | 0.437s | 1.457ns  |


### Analysis

The measurments clearly show that on this system function calls are the most expensive indirect jumps. As their execution time per command is significantly and consistantly higher that for the others. 

This may be because function calls often involve additional overhead such as stack frame setup and teardown, which can add to the execution time. Furthermore depending on the calling convention used by the compiler, function calls may also require saving and restoring registers, which can further increase the execution time.

Variant B, which uses a switch statement, is faster than variant A. This may be because switch statements can be optimized by the compiler to use jump tables, which allow for faster branching compared to multiple if-else statements. Furthermore the switch statement is no function call, which further reduces the overhead.

The fastests variant was the C variant, which uses a goto approach. Bascially what it does is thaat an array is initialized with the addresses of the labels. Then an indirect jump is performed to the address stored in the array, the command executed and then another jump to the next label is performed. This approach is very fast, as it only requires a single indirect jump without any additional overhead.

### TLDR
C is fast because there is only one indirect jump per command. B is faster than A because switch statements can be optimized as a jump table and avoid function call overhead.

> **Note**: The measurements are highly sensitive to the hardware and compiler optimizations. Different systems or compiler settings may yield different results.


## b) Testing for direct jumps

### Measurement on Milan
| Var-Seq | size | total  | perCmd   |
|---------|------|--------|----------|
| A-fix1  | 200  | 0.701s | 2.337ns  |
| A-fix2  | 200  | 0.701s | 2.337ns  |
| B-fix1  | 200  | 0.537s | 1.791ns  |
| B-fix2  | 200  | 0.538s | 1.792ns  |
| C-fix1  | 200  | 0.435s | 1.451ns  |
| C-fix2  | 200  | 0.438s | 1.459ns  |
| D-fix1  | 200  | 0.183s | 0.609ns  |
| D-fix2  | 200  | 0.187s | 0.623ns  |

### Analysis
The measurments show that direct jumps (done by using if-chains) are significantly faster than indirect jumps. This is because direct jumps have a fixed target address, which allows the CPU to predict and optimize the jump more effectively. 

Variant D is semantically very similar to variant B, basically the only difference is that instead of using a switch variant D uses nested if-else statements to determine the next command to execute. This however allows the branch predictor to work more effectively, because most branches are heavily biased so after only a few iterations the predictor very accurately can correctly predict most branches. Due to the low amount of different cases the swich statement in variant B cant be effectively optimized as a jump table. For more cases or more randomized distribution over the if else statements the switch my be faster. 


## c) Predictors for return instructions

### Measurement on Milan
| Var-Seq   | size | total  | perCmd   |
|-----------|------|--------|----------|
| A-fix1    | 200  | 0.700s | 2.334ns  |
| A-fix2    | 200  | 0.704s | 2.345ns  |
| E1-fix1   | 200  | 0.875s | 2.916ns  |
| E1-fix2   | 200  | 0.875s | 2.917ns  |
| E6-fix1   | 200  | 0.451s | 1.503ns  |
| E6-fix2   | 200  | 0.452s | 1.505ns  |
| E11-fix1  | 200  | 0.436s | 1.452ns  |
| E11-fix2  | 200  | 0.439s | 1.462ns  |
| E16-fix1  | 200  | 0.442s | 1.472ns  |
| E16-fix2  | 200  | 0.441s | 1.469ns  |
| E21-fix1  | 200  | 0.443s | 1.477ns  |
| E21-fix2  | 200  | 0.443s | 1.478ns  |
| E106-fix1 | 200  | 0.468s | 1.562ns  |
| E106-fix2 | 200  | 0.469s | 1.563ns  |

### Analysis
The measurments show that using a return address stack significantly improves the performance of function calls and returns. The fastest variant is E11 (G) or E16 (H), which uses a depth of 11 or 16 respectively. Indicating that the return address stack can store all of these adresses. When going beyond (H) or before (G) the performance degrades again, as the stack either cant store all return addresses or is underutilized. Depending on the run E11 or E16 may be slightly faster, but the difference is negligible and likely due to measurement noise. Therefore we can conclude that the size of the stack is close to this range.

## Sources

- [Basic information about branch prediction]( https://johnfarrier.com/branch-prediction-the-definitive-guide-for-high-performance-c/)

- [Jump table & branch target predictor](https://stackoverflow.com/questions/11668090/how-to-deal-with-branch-prediction-when-using-a-switch-case-in-cpu-emulation)
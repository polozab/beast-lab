# Task 02 - Measuring Instruction Latency and Throughput

## a)

### i.)
The `CHAINS` parameter controls how many independent instruction chains exist. Therefor the innermost loop computes exactly `CHAINS` amount of partial results. This allows the CPU to pipeline `CHAINS` number of instruction as they don't have data dependencies between them anymore.

Looking at the assembly code for ARM64 with `CHAINS 1` we get the serial execution with one single instruction chain, were each `i`-iteration executes exactly one `OP`-instruction. Here an example for `OP +` (add):

~~~asm
add     x1, x1, 1
add     x20, x20, x19
mov     x19, 3
cmp     x21, x1
bne     .L3
~~~

The same holds for x86, however we choose ARM, as it is more clear to see that different source register get used. 

If we instead choose a larger `CHAINS` we get exactly `CHAINS` number of `OP`-instruction executed per `i`-loop iteration, as between these partial results no dependency exists. This technique is called loop unrolling, as we perform several iterations of the loop in parallel. To make sure the compiler will attempt to unroll the loop we additionally include the compiler pragma `#pragma GCC unroll 65534` which tells GCC to try to unroll the loop into 65534 iterations to one. As the compiler is only able to unroll `CHAINS` amount of instructions, due to the previously mentioned data dependencies, it will settle at exactly this `CHAINS` value for unrolling.

~~~asm
add     x21, x21, 1
add     x20, x20, x19
add     x0, x0, x19
add     x2, x2, x19
add     x1, x1, x19
add     x3, x3, x19
add     x4, x4, x19
add     x5, x5, x19
add     x7, x7, x19
add     x6, x6, x19
add     x10, x10, x19
add     x8, x8, x19
add     x13, x13, x19
add     x9, x9, x19
add     x15, x15, x19
add     x11, x11, x19
add     x16, x16, x19
add     x12, x12, x19
add     x17, x17, x19
add     x14, x14, x19
add     x18, x18, x19
mov     x19, 3
cmp     x21, x23
bne     .L10
cmp     x30, x21
bls     .L6
~~~

### ii)

Looking again at the last assembly code we can see that the add instructions for the `i`-loop are using different register operands. In `ARM64` the register `ADD`-operation uses three register operands. The first one sets the destination register, while the last two are the registers that hold the two addends of the addition.

We can see that the compiler uses different registers to store operands for the `ADD`-instruction.
The `x19` register is the constant `val2` which gets added to every register `x0`-`x19` that are used to save the individual partial sums of each independent `ADD`-chain. We can therefor deduce that at least `CHAINS` was set to 20, which indeed was the case.

If we instead use `CHAINS 5` we see that first off all only 5 additions appear and secondly for these additions only 5 different register get used as operands:

~~~asm
.L27:
add     x4, x4, 1       # Loop counter increment
add     x20, x20, x19   # Begin of add-instructions
add     x0, x0, x19
add     x2, x2, x19
add     x1, x1, x19
add     x3, x3, x19     # End of add-instructions
mov     x19, 3
cmp     x4, x6          # Comparison against condition of i-loop
bne     .L27
cmp     x7, x4
bls     .L6
~~~

For even more chains (e.g. `CHAINS 32`) we get:

~~~asm
.L5:
ldr     x1, [sp, 120]
add     x0, x0, 1
add     x20, x20, x19
add     x27, x27, x19
add     x1, x1, x19
str     x1, [sp, 120]
ldr     x1, [sp, 112]
add     x26, x26, x19
add     x25, x25, x19
add     x24, x24, x19
add     x1, x1, x19
str     x1, [sp, 112]
ldr     x1, [sp, 104]
add     x23, x23, x19
add     x22, x22, x19
add     x21, x21, x19
add     x1, x1, x19
str     x1, [sp, 104]
ldr     x1, [sp, 96]
add     x30, x30, x19
add     x18, x18, x19
add     x17, x17, x19
add     x1, x1, x19
str     x1, [sp, 96]
ldr     x1, [sp, 128]
add     x16, x16, x19
add     x15, x15, x19
add     x14, x14, x19
add     x1, x1, x19
str     x1, [sp, 128]
ldr     x1, [sp, 136]
add     x13, x13, x19
add     x12, x12, x19
add     x11, x11, x19
add     x10, x10, x19
add     x9, x9, x19
add     x8, x8, x19
add     x7, x7, x19
add     x6, x6, x19
add     x5, x5, x19
add     x28, x28, x19
add     x4, x4, x19
add     x3, x3, x19
add     x2, x2, x19
mov     x19, 3
cmp     x0, x1
bne     .L5
~~~

Now we can spot that our architectural register are fully used which means that the compiler outsources some of the partial sums to the memory. For that it uses the `str` (store register) and `ldr` (load register) memory instructions which we can see throughout the code snippet of one iteration. This ensures that the given number for unrolling the loop still gets reached and we make use of the larger number of independent add chains. We can also observe that the compiler executes these memory operations with a latency of three cycles each. We think the compiler tries to optimize for the out-of-order execution of the processor to effectively eliminate the three stall cycles of a L1 cache access to increase the throughput.

### iii.)

To measure the possible throughput of an operation we have to at least match the independent instructions chains with `CHAINS` and the number of available functional units dedicated to this instruction. If we chose a too small amount of `CHAINS`, we can get limited by the latency of the `OP`-instruction as the data dependency forbids the CPU to use further functional units. If instead we use a too large number for `CHAINS` we get limited by the latency of load and store operations. In that case we get the unwanted memory access latency which could slow down the measured IPC. Thus setting `CHAINS` to a sufficiently high number to saturate all functional units and is small enough so no spilling to the memory occurs is very important. Only than is the throughput of the functional units the sole limiting factor that tells us the actual throughput of the instruction.

## b)

To perform the throughput calculations we chose to use likwid to measure the average clock frequency while executing our test loop. We noticed that for every operation and on every system the performance reached almost the max boost clock frequency specified by the vendor. This is expected as we only utilize one core and therefor were not limited by power or thermals. We only utilize parallelism is on an instruction level depending on the amount of independent instruction chains.

### i.)

To compute the instruction latency and throughput we have to take a look at results of different numbers of `CHAINS`. Specifically for the instruction latency we have to make sure only a single instruction chain can get executed so we receive sequential results for execution time. And if we want to measure the instruction throughput we have to make sure that we utilize all available execution ports, while not spilling over registers to the memory, which would reduce throughput.


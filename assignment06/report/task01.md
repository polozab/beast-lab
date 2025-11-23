# Task 01 - Understanding Instruction Latency and Throughput

## a.)

### i.)

As only the addition has the dependency between loop iteration, where each iteration needs the temporary value from the last iteration to add up. Thus the loop runs at exactly the add latency.
The multiply operation on the other side, operates on new values each iteration and doesn't depend on previous iterations. This means the multiply can be pipelined making it impossible to deduce the instruction latency as we only observe the multiply throughput.

### ii.)

We can run the dot product operation for a large `N` and measure the execution time. As every add instruction in the loop gets executed sequentially the cycles needed for every iteration closely represent the latency of an add instruction. Thus:
~~~
add_latency = exec_time * clock_freq / N
~~~

### iii.)

To enable pipelining we could unroll the loop, so that for all pre-flight add instructions there exist no WAR dependencies. Let `X` denote the latency in cycles for one add instruction, then we could use:

~~~C
double sum1 = sum2 = ... = sumX = 0.0;

for (int i = 0; i < N; i += X)
{
  sum1 = a[i] * b[i];
  sum2 = a[i + 1] * b[i + 1];
  ...
  sumX = a[i + X] * b[i + X];
}

double sum = sum1 + sum2 + ... + sumX;
~~~

Now the dependency still exist between each iteration. However, as they appear not for in-flight instructions they don't stall the pipeline anymore.

## b.)

### i.)

As we saw in the last task you need as many independent instruction chains as your given instruction latency. Thus we need exactly `n` chains to fully utilize the pipeline.

### ii.)

Now that the core makes use of `m` execution ports for the instruction it can also potentially handle `m` instructions in parallel. As the dependency still exists between every instruction, we have to increase the number of instruction chains by multiplying `m * n` to fully utilize the pipeline.

### iii.)

If a core offers multiple execution port for an instruction and the latency of that instruction is smaller as the amount of execution ports, we can theoretically issue new instructions before older ones were finished. Thus our throughput is higher than one and so the reciprocal is smaller than one. 

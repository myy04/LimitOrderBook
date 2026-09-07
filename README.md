# Limit Order Book (LOB)

A Limit Order Book implementation featuring both Python and C++ cores, designed for efficient order matching and market data snapshotting.
(python bindings do no work in this branch (refer to main))

# Performance

```
Unable to determine clock rate from sysctl: hw.cpufrequency: No such file or directory
This does not affect benchmark measurements, only the metadata output.
***WARNING*** Failed to set thread affinity. Estimated CPU frequency may be incorrect.
2026-09-08T01:27:07+05:00
Running ./build/bin/benchmark_cpp
Run on (10 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x10)
Load Average: 2.39, 2.04, 1.92
---------------------------------------------------------------------------
Benchmark                 Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------
BM_1000_Orders       101715 ns       101709 ns       137714 items_per_second=9.83202M/s
BM_10000_Orders      954299 ns       953824 ns        14864 items_per_second=10.4841M/s
BM_100000_Orders   12054415 ns     12050542 ns         1167 items_per_second=8.29838M/s
```

## Benchmark setup
-- This performance test has been done using google benchmark.\
-- Pre-generated 1K/10K/100K valid order requests.\
-- Compiled with ```-03``` flag\
-- Snapshotting is turned off (```CONFIG::CAPTURE_SNAPSHOTS = false``` in Config.h)\
-- Apple Silicon M4 (16GB RAM)




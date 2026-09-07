# Limit Order Book (LOB)

A Limit Order Book implementation featuring both Python and C++ cores, designed for efficient order matching and market data snapshotting.
(python bindings do no work in this branch (refer to main))

# Performance

```
$ ./build/bin/benchmark_cpp --benchmark_min_time=10s                                                          [0:48:23]
Unable to determine clock rate from sysctl: hw.cpufrequency: No such file or directory
This does not affect benchmark measurements, only the metadata output.
***WARNING*** Failed to set thread affinity. Estimated CPU frequency may be incorrect.
2026-09-08T00:48:27+05:00
Running ./build/bin/benchmark_cpp
Run on (10 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x10)
Load Average: 2.77, 3.01, 2.77
------------------------------------------------------------------------
Benchmark              Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------
BM_1000Orders     973596 ns       973576 ns        14376 items_per_second=1.02714M/s
```

## Performance setup
-- This performance test has been done using google benchmark.\
-- Pre-generated 10000 valid order requests.\
-- Compiled with ```-03``` flag\
-- Snapshotting is turned off (```CONFIG::CAPTURE_SNAPSHOTS = false``` in Config.h)\
-- Apple Silicon M4 (16GB RAM)

## Result

In this setup, this implementation shows ~1M orders per second.
Results may very depending on the setup. For example, if the amount of orders is increased without resetting the orderbook, the performance worsens. 




# Limit Order Book (LOB)

A Limit Order Book implementation featuring both Python and C++ cores, designed for efficient order matching and market data snapshotting. 

# Performance

```
$ ./build/bin/benchmark_cpp --benchmark_min_time=10s                                                          
Unable to determine clock rate from sysctl: hw.cpufrequency: No such file or directory
This does not affect benchmark measurements, only the metadata output.
***WARNING*** Failed to set thread affinity. Estimated CPU frequency may be incorrect.
2026-09-09T01:42:31+05:00
Running ./build/bin/benchmark_cpp
Run on (10 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x10)
Load Average: 2.16, 2.45, 2.30
---------------------------------------------------------------------------------
Benchmark                       Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------
BM_Orders_Tails/1000       123979 ns       123305 ns       116354 items_per_second=8.10994M/s max_ns=371.458k p50_ns=83 p90_ns=125 p99.9_ns=3.041k p99_ns=209
BM_Orders_Tails/10000     1117313 ns      1113336 ns        12489 items_per_second=8.98202M/s max_ns=156.5k p50_ns=83 p90_ns=125 p99.9_ns=708 p99_ns=167
BM_Orders_Tails/100000   13769459 ns     13702368 ns         1042 items_per_second=7.29801M/s max_ns=16.6854M p50_ns=84 p90_ns=167 p99.9_ns=583 p99_ns=292
```

## Benchmark setup
-- This performance test has been done using google benchmark.\
-- Pre-generated 1K/10K/100K valid order requests.\
-- Compiled with ```-03``` flag\
-- Snapshotting is turned off (```CONFIG::CAPTURE_SNAPSHOTS = false``` in Config.h)\
-- Apple Silicon M4 (16GB RAM)




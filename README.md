# Limit Order Book (LOB)

A Limit Order Book implementation with a C++ core exposed through a Python bindings layer, designed for efficient order matching and market data snapshotting. `OrderGateway` is the single entry point for clients.

# Performance

```
$ ./build/bin/benchmark_cpp --benchmark_min_time=10s                                                                                                                                                      [19:33:24]
Unable to determine clock rate from sysctl: hw.cpufrequency: No such file or directory
This does not affect benchmark measurements, only the metadata output.
***WARNING*** Failed to set thread affinity. Estimated CPU frequency may be incorrect.
2026-09-09T19:33:29+05:00
Running ./build/bin/benchmark_cpp
Run on (10 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x10)
Load Average: 3.23, 4.07, 3.70
---------------------------------------------------------------------------------
Benchmark                       Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------
BM_Orders_Tails/1000       116251 ns       116085 ns       120708 items_per_second=8.61436M/s max_ns=567.905k mean_ns=85.3815 p50_ns=84 p90_ns=135 p99.9_ns=365 p99_ns=249 timer_overhead_ns=12
BM_Orders_Tails/10000     1309083 ns      1308495 ns        10579 items_per_second=7.64237M/s max_ns=575.488k mean_ns=100.594 p50_ns=95 p90_ns=154 p99.9_ns=401 p99_ns=279 timer_overhead_ns=12
BM_Orders_Tails/100000   14520594 ns     14520252 ns          961 items_per_second=6.88693M/s max_ns=141.571k mean_ns=115.053 p50_ns=97 p90_ns=219 p99.9_ns=797 p99_ns=505 timer_overhead_ns=12
```

## Benchmark setup
-- This performance test has been done using google benchmark.\
-- Pre-generated 1K/10K/100K valid order requests, reshuffled before each iteration.\
-- Compiled with ```-03``` flag\
-- Snapshotting is turned off (```CONFIG::CAPTURE_SNAPSHOTS = false``` in Config.h)\
-- Apple Silicon M4 (16GB RAM)

## Measurement methodology
-- Latency percentiles (p50/p90/p99/p99.9) computed with a streaming histogram (32 ns buckets up to 2 us, power-of-two buckets beyond), so memory stays constant regardless of run length.\
-- Timer overhead (```steady_clock::now()``` pair) is calibrated up front and subtracted from each sample; the measured overhead (~12 ns) is reported in the output.\
-- Book state is reset in place between iterations to avoid allocator churn and page faults leaking into timed samples.




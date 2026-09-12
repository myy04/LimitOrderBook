# Limit Order Book (LOB)

A Limit Order Book implementation with a C++ core exposed through a Python bindings layer, designed for efficient order matching and market data snapshotting. `OrderGateway` is the single entry point for clients.

# C++ Performance

```
---------------------------------------------------------------------------------
Benchmark                       Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------
BM_Orders_Tails/1000       112010 ns       111997 ns         6209 items_per_second=8.92878M/s max_ns=12.406k mean_ns=84.2284 p50_ns=72 p90_ns=155 p99.9_ns=323 p99_ns=239 timer_overhead_ns=11
BM_Orders_Tails/10000     1301367 ns      1291187 ns          498 items_per_second=7.74481M/s max_ns=584.029k mean_ns=100.988 p50_ns=71 p90_ns=154 p99.9_ns=487 p99_ns=320 timer_overhead_ns=13
BM_Orders_Tails/100000   13981973 ns     13981522 ns           46 items_per_second=7.1523M/s max_ns=123.155k mean_ns=111.847 p50_ns=72 p90_ns=197 p99.9_ns=780 p99_ns=488 timer_overhead_ns=12
```

## Benchmark setup
-- This performance test has been done using google benchmark.\
-- Pre-generated 1K/10K/100K valid order requests, reshuffled before each iteration.\
-- Compiled with ```-03``` flag\
-- Snapshotting is turned off for benchmarking\
-- Apple Silicon M4 (16GB RAM)


# Python Bindings

## Usage

```
pip install -e .
```

Quick start:

```python
from LimitOrderBook import OrderGateway, OrderRequest, OrderSide, Mpid

gateway = OrderGateway()
result = gateway.submit_order(OrderRequest(side=OrderSide.BUY, price=100.0, volume=10, trader_id=Mpid("TEST")))
```

Run the demo (live order feed):

```
python src/main.py
```


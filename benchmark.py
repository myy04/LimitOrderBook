import LimitOrderBook as lob
from OrderGenerator import OrderGenerator 

import time
import json
import gc
import statistics

def run_benchmark(engine, orders, iterations=5):
    order_gateway = lob.OrderGateway(engine=engine)
    times = []
    
    # Warm-up
    for order in orders[:1000]:
        try:
            order_gateway.submit_order_request(order)
        except lob.OrderException:
            pass

    for _ in range(iterations):
        gc.collect()
        gc.disable()
        t0 = time.perf_counter()
        for order in orders:
            try:
                order_gateway.submit_order_request(order)
            except lob.OrderException:
                continue
        t1 = time.perf_counter() - t0
        gc.enable()
        times.append(t1)
        if len(orders) >= 1e5: # Only run once for large datasets
            break
    
    median_time = statistics.median(times)
    throughput = len(orders) / median_time if median_time > 0 else 0
    return median_time, throughput

if __name__ == "__main__":
    results = []
    test_sizes = [int(1e2), int(1e4), int(1e5)]
    
    for num_orders in test_sizes:
        print(f"Orders: {num_orders}")
        
        # Pre-generate orders to ensure both implementations use the same data
        order_generator = OrderGenerator()
        orders = []
        for order in order_generator:
            orders.append(order)
            if len(orders) == num_orders:
                break
        
        # C++ Benchmark
        cpp_engine = lob.cpp.MatchingEngine()
        cpp_time, cpp_tps = run_benchmark(cpp_engine, orders)
        print(f"CPP Time: {cpp_time:.6f}s, Throughput: {cpp_tps:,.2f} orders/sec")
        
        # Python Benchmark
        py_engine = lob.MatchingEngine()
        py_time, py_tps = run_benchmark(py_engine, orders)
        print(f"Python Time: {py_time:.6f}s, Throughput: {py_tps:,.2f} orders/sec")
        
        ratio = py_time / cpp_time
        print(f"Python/CPP ratio: {ratio:.4f}")

        results.append({
            "num_orders": num_orders,
            "cpp": {"time": cpp_time, "throughput": cpp_tps},
            "python": {"time": py_time, "throughput": py_tps},
            "ratio": ratio
        })
        print("-" * 100)

    with open("benchmark_results.json", "w") as f:
        json.dump(results, f, indent=4)

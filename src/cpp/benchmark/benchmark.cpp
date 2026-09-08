#include <benchmark/benchmark.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <memory>

#include "../include/lob/OrderGenerator.h"
#include "../include/lob/OrderGateway.h"
#include "../include/lob/MatchingEngine.h"

static void BM_Orders_Tails(benchmark::State& state) {
    const size_t num_orders = static_cast<size_t>(state.range(0));
    
    auto engine = std::make_shared<MatchingEngine>();
    auto gateway = std::make_unique<OrderGateway>(engine);
    auto generator = OrderGenerator{2};

    std::vector<OrderGateway::OrderRequest> orders(num_orders);
    for (size_t i = 0; i < num_orders; i++) {
        orders[i] = generator.generate_order();
    }

    // Pre-allocate memory to avoid reallocation overhead during the timed loop
    std::vector<uint64_t> latencies_ns;
    latencies_ns.reserve(num_orders * 10); 

    for (auto _ : state) {
        engine->reset(); 
        
        for (const auto& request : orders) {
            auto start = std::chrono::steady_clock::now();
            try {
                gateway->submit_order(request);
            } catch (const GatewayException&) {}
            auto end = std::chrono::steady_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            latencies_ns.push_back(static_cast<uint64_t>(duration));
        }
    }

    // Calculate percentiles after the benchmark loop completes
    if (!latencies_ns.empty()) {
        std::sort(latencies_ns.begin(), latencies_ns.end());

        auto percentile = [&](double p) -> double {
            size_t idx = static_cast<size_t>(p * (latencies_ns.size() - 1));
            return static_cast<double>(latencies_ns[idx]);
        };

        state.counters["p50_ns"]   = percentile(0.50);
        state.counters["p90_ns"]   = percentile(0.90);
        state.counters["p99_ns"]   = percentile(0.99);
        state.counters["p99.9_ns"] = percentile(0.999);
        state.counters["max_ns"]   = static_cast<double>(latencies_ns.back());
    }

    state.SetItemsProcessed(state.iterations() * num_orders);
}

BENCHMARK(BM_Orders_Tails)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();
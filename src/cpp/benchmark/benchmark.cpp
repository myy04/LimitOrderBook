#include <benchmark/benchmark.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <memory>
#include <numeric>
#include <random>

#include "../include/lob/OrderGenerator.h"
#include "../include/lob/OrderGateway.h"

static double percentile(const std::vector<uint64_t>& sorted_ns, double p) {
    if (sorted_ns.empty()) return 0.0;
    double pos = p * (sorted_ns.size() - 1);
    size_t lower = static_cast<size_t>(pos);
    size_t upper = lower + 1;
    if (upper >= sorted_ns.size()) return static_cast<double>(sorted_ns.back());
    double frac = pos - lower;
    return static_cast<double>(sorted_ns[lower]) * (1.0 - frac) +
           static_cast<double>(sorted_ns[upper]) * frac;
}

static uint64_t calibrate_timer_overhead_ns() {
    constexpr int K = 10000;
    std::chrono::steady_clock::time_point acc{};
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < K; i++) {
        acc = std::chrono::steady_clock::now();
    }
    auto t1 = std::chrono::steady_clock::now();
    benchmark::DoNotOptimize(acc);
    auto total = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    return static_cast<uint64_t>(total / K);
}

static void BM_Orders_Tails(benchmark::State& state) {
    const size_t num_orders = static_cast<size_t>(state.range(0));

    auto gateway = std::make_unique<OrderGateway>();
    auto generator = OrderGenerator{2};

    std::vector<OrderRequest> orders(num_orders);
    for (size_t i = 0; i < num_orders; i++) {
        orders[i] = generator.generate_order();
    }

    const uint64_t timer_overhead = calibrate_timer_overhead_ns();

    std::vector<uint64_t> latencies;
    latencies.reserve(state.max_iterations * num_orders);

    std::mt19937_64 shuffle_gen(12345);

    for (auto _ : state) {
        state.PauseTiming();
        gateway->reset();
        std::shuffle(orders.begin(), orders.end(), shuffle_gen);
        state.ResumeTiming();

        for (const auto& request : orders) {
            auto start = std::chrono::steady_clock::now();
            MatchResult result;
            try {
                result = gateway->submit_order(request);
            } catch (const GatewayException&) {
            }
            auto end = std::chrono::steady_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            uint64_t sample = (duration > timer_overhead) ? static_cast<uint64_t>(duration - timer_overhead) : 0;
            latencies.push_back(sample);

            benchmark::DoNotOptimize(result);
        }
    }

    std::sort(latencies.begin(), latencies.end());

    state.counters["p50_ns"]   = percentile(latencies, 0.50);
    state.counters["p90_ns"]   = percentile(latencies, 0.90);
    state.counters["p99_ns"]   = percentile(latencies, 0.99);
    state.counters["p99.9_ns"] = percentile(latencies, 0.999);
    state.counters["max_ns"]   = latencies.empty() ? 0.0 : static_cast<double>(latencies.back());
    state.counters["mean_ns"]  = latencies.empty() ? 0.0 : static_cast<double>(std::accumulate(latencies.begin(), latencies.end(), 0)) / latencies.size();
    state.counters["timer_overhead_ns"] = static_cast<double>(timer_overhead);

    state.SetItemsProcessed(state.iterations() * num_orders);
}

BENCHMARK(BM_Orders_Tails)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();

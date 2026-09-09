#include <benchmark/benchmark.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <memory>
#include <numeric>
#include <random>

#include "../include/lob/OrderGenerator.h"
#include "../include/lob/OrderGateway.h"

// Streaming latency histogram:
//  -- 32 ns-wide buckets cover 0..2 us (where p50/p90/p99/p99.9 live)
//  -- power-of-two buckets cover 2 us..16.7 ms for the tail
// No per-sample storage, so memory stays constant regardless of run length.
class LatencyHistogram {
public:
    void record(uint64_t ns) {
        size_t bucket = bucket_for(ns);
        if (bucket >= counts_.size()) counts_.resize(bucket + 1, 0);
        counts_[bucket]++;
        total_++;
        sum_ns_ += ns;
        max_ns_ = std::max(max_ns_, ns);
    }

    // Percentile via cumulative counts with linear interpolation inside the bucket.
    double percentile(double p) const {
        if (total_ == 0) return 0.0;
        uint64_t target = static_cast<uint64_t>(p * static_cast<double>(total_ - 1));
        uint64_t seen = 0;
        for (size_t i = 0; i < counts_.size(); i++) {
            if (counts_[i] == 0) continue;
            if (seen + counts_[i] > target) {
                uint64_t lower = lower_bound_ns(i);
                uint64_t width = bucket_width_ns(i);
                uint64_t offset = (target - seen) * width / counts_[i];
                return static_cast<double>(lower + offset);
            }
            seen += counts_[i];
        }
        return static_cast<double>(max_ns_);
    }

    double mean_ns() const { return total_ ? static_cast<double>(sum_ns_) / total_ : 0.0; }
    uint64_t max_ns() const { return max_ns_; }
    uint64_t samples() const { return total_; }

    static constexpr uint64_t FINE_BUCKET_WIDTH = 32; // ns
    static constexpr uint64_t FINE_MAX = 2048;         // ns, end of fine range

    static uint64_t bucket_width_ns(size_t bucket) {
        if (bucket < FINE_BUCKETS) return FINE_BUCKET_WIDTH;
        uint64_t power = 1ull << (bucket - FINE_BUCKETS + 1);
        return power * FINE_BUCKET_WIDTH;
    }

private:
    static constexpr size_t FINE_BUCKETS = FINE_MAX / FINE_BUCKET_WIDTH; // 64

    static uint64_t lower_bound_ns(size_t bucket) {
        if (bucket < FINE_BUCKETS) return bucket * FINE_BUCKET_WIDTH;
        uint64_t lower = FINE_MAX;
        for (size_t i = FINE_BUCKETS; i < bucket; i++) lower += bucket_width_ns(i);
        return lower;
    }

    size_t bucket_for(uint64_t ns) const {
        if (ns < FINE_MAX) return ns / FINE_BUCKET_WIDTH;
        uint64_t lower = FINE_MAX;
        size_t bucket = FINE_BUCKETS;
        while (ns >= lower + bucket_width_ns(bucket)) {
            lower += bucket_width_ns(bucket);
            bucket++;
        }
        return bucket;
    }

    std::vector<uint64_t> counts_;
    uint64_t total_ = 0;
    uint64_t sum_ns_ = 0;
    uint64_t max_ns_ = 0;
};

// Measure the overhead of a single steady_clock::now() call by timing a
// batch of back-to-back reads and amortizing; the min-pair method only
// captures clock granularity, not call cost.
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

    // Pre-generate the order batch once (untimed); each iteration replays a
    // fresh shuffle of it so branch predictors are not trained on one sequence.
    std::vector<OrderGateway::OrderRequest> orders(num_orders);
    for (size_t i = 0; i < num_orders; i++) {
        orders[i] = generator.generate_order();
    }

    const uint64_t timer_overhead = calibrate_timer_overhead_ns();

    LatencyHistogram hist;
    std::mt19937_64 shuffle_gen(0x5EED);

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
            hist.record(sample);

            benchmark::DoNotOptimize(result);
        }
    }

    state.counters["p50_ns"]   = hist.percentile(0.50);
    state.counters["p90_ns"]   = hist.percentile(0.90);
    state.counters["p99_ns"]   = hist.percentile(0.99);
    state.counters["p99.9_ns"] = hist.percentile(0.999);
    state.counters["max_ns"]   = static_cast<double>(hist.max_ns());
    state.counters["mean_ns"]  = hist.mean_ns();
    state.counters["timer_overhead_ns"] = static_cast<double>(timer_overhead);

    state.SetItemsProcessed(state.iterations() * num_orders);
}

BENCHMARK(BM_Orders_Tails)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();
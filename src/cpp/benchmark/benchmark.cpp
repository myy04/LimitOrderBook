#include <benchmark/benchmark.h>
#include <memory>
#include <array>

#include "../include/lob/OrderGenerator.h"
#include "../include/lob/OrderGateway.h"
#include "../include/lob/MatchingEngine.h"

static void BM_1000Orders(benchmark::State& state) {
    // Setup (once per benchmark, not per iteration)
    auto engine = std::make_shared<MatchingEngine>();
    auto gateway = std::make_unique<OrderGateway>(engine);
    auto generator = OrderGenerator{2};


    std::array<OrderGateway::OrderRequest, 1000> orders;
    for (size_t i = 0; i < orders.size(); i++) {
        orders[i] = generator.generate_order();
    }

    for (auto _ : state) {
        engine->reset(); 

        for (const auto& request : orders) {
            try {
                gateway->submit_order(request);
            } catch (const GatewayException&) {
            }
        }
    }

    state.SetItemsProcessed(state.iterations() * 1000);
}

BENCHMARK(BM_1000Orders);

BENCHMARK_MAIN();

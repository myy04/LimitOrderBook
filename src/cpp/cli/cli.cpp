#include <iostream>
#include <thread>
#include <atomic>

#include "../include/lob/OrderGenerator.h"
#include "../include/lob/MatchingEngine.h"
#include "../include/lob/OrderGateway.h"

std::atomic<bool> running{true};

void output_snapshot(const BookSnapshot& snap) {
    system("clear");

    std::cout << snap.time << std::endl;

    std::cout << "Bids:" << std::endl;
    for (const Order& bid : snap.bids) {
        std::cout << bid << std::endl;
    }

    std::cout << "Asks:" << std::endl;
    for (const Order& ask : snap.asks) {
        std::cout << ask << std::endl;
    }    

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main() {
    signal(SIGINT, [](int sig){running.store(false);});

    auto engine = std::make_shared<MatchingEngine>();
    auto gateway = OrderGateway(engine);
    auto generator = OrderGenerator{2};

    std::thread([&]() {
        while (running.load()) {
            output_snapshot(engine->pull_snapshot());
        }
    }
    ).detach();
        
    while (running.load()) {
        try {
            gateway.submit_order(generator.generate_order());
        } catch (GatewayException) {
            continue;
        }
    }



    return 0;
}

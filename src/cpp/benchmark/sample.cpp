#include <iostream>
#include <memory>
#include <array>

#include "../include/lob/OrderGenerator.h"
#include "../include/lob/OrderGateway.h"

int main() {
    auto gateway = std::make_unique<OrderGateway>();
    auto generator = OrderGenerator{2};

    std::array<OrderRequest, 20> orders;
    for (size_t i = 0; i < orders.size(); i++) {
        orders[i] = generator.generate_order();
    }

    for (const auto& request : orders) {
        std::cout << "Order: " << request.trader_id << ' ' << request.side << ' ' << request.price << ' ' << request.volume << ' ';
        MatchResult result = gateway->submit_order(request);
        const auto& trades = result.trades;  
        const auto& cancellations = result.cancellations;

        for (const auto& trade : trades) {
            std::cout << "Trade:\n";
            std::cout << "Resting Order: " << trade.resting_order_id << '\n';
            std::cout << "Agressor Order: " << trade.aggressor_order_id << '\n';
            std::cout << "Price: " << trade.price << '\n';
            std::cout << "Volume: " << trade.volume << '\n';
        }

        for (const auto& cancel : cancellations) {
            std::cout << "Self Cancellation:\n";
            std::cout << "Resting Order: " << cancel.resting_order_id << '\n';
            std::cout << "Agressor Order: " << cancel.aggressor_order_id << '\n';
            std::cout << "Price: " << cancel.price << '\n';
            std::cout << "Volume: " << cancel.volume << '\n';
        }
    }

    return 0;
}

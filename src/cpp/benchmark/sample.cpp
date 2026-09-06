#include <iostream>
#include <memory>
#include <array>

#include "../include/lob/OrderGenerator.h"
#include "../include/lob/OrderGateway.h"
#include "../include/lob/MatchingEngine.h"

int main() {
    auto engine = std::make_shared<MatchingEngine>();
    auto gateway = std::make_unique<OrderGateway>(engine);
    auto generator = OrderGenerator{2};


    std::array<OrderGateway::OrderRequest, 1000> orders;
    for (size_t i = 0; i < orders.size(); i++) {
        orders[i] = generator.generate_order();
    }

    for (const auto& request : orders) {
        std::cout << request.trader_id << ' ' << request.side << ' ' << request.price << ' ' << request.volume << ' ';

        try {
            gateway->submit_order(request);
            std::cout << "VALID\n";
        } catch (const GatewayException& e) {
            std::cout << "INVALID " << e.what() << '\n';
        }
    }


    return 0;
}

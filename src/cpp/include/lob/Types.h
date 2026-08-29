#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

enum class OrderSide {BUY, SELL, UNDEFINED};

struct Order {
    OrderSide side{OrderSide::UNDEFINED};
    int price{};
    int volume{};
    int order_id{};
    float timestamp{};
    std::string trader_id{};
};

struct Trade {
    Order aggressor_order{};
    Order resting_order{};
    int price{};
    int volume{};
};

struct SelfTradeCancellation {
    Order aggressor_order{};
    Order resting_order{};
    int volume{};
    int price{};
};

struct MatchResult {
    std::vector<Trade> trades{};
    std::vector<SelfTradeCancellation> cancellations{};
};

struct BookSnapshot {
    std::vector<Order> bids{};
    std::vector<Order> asks{};
    float time{};
};

#endif //TYPES_H
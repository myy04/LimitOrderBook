#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>
#include <iostream>
#include <chrono>

enum class OrderSide {BUY, SELL, UNDEFINED};

inline std::ostream& operator<<(std::ostream& os, OrderSide side) {
    switch (side) {
        case OrderSide::BUY: return os << "BUY";
        case OrderSide::SELL: return os << "SELL";
        case OrderSide::UNDEFINED: return os << "UNDEFINED";
    }
}

struct Mpid {
    uint32_t tag;

    Mpid(): tag{0} {}

    Mpid(const char* s): tag{0} {
        for (int i = 0; i < 4; i++) {
            tag <<= 8;
            tag |= static_cast<uint32_t>(s[i]);
        }
    }

    const bool operator==(const Mpid& other) const {return tag == other.tag;}
};

inline std::ostream& operator<<(std::ostream& os, Mpid id) {    
    char s[5];
    s[0] = static_cast<char>((id.tag >> 24) & 0xFF);  // 'C'
    s[1] = static_cast<char>((id.tag >> 16) & 0xFF);  // 'D'
    s[2] = static_cast<char>((id.tag >>  8) & 0xFF);  // 'E'
    s[3] = static_cast<char>( id.tag        & 0xFF);  // 'L'
    return os << s;
} 

struct Order {
    OrderSide side;
    size_t price;
    size_t volume;
    size_t order_id;
    Mpid trader_id;
    std::chrono::milliseconds timestamp;

    using side_t = decltype(side);
    using price_t = decltype(price);
    using volume_t = decltype(volume);
    using order_id_t = decltype(order_id);
    using trader_id_t = decltype(trader_id);
    using timestamp_t = decltype(timestamp);
};

struct Trade {
    Order aggressor_order;
    Order resting_order;
    int price;
    int volume;
};

struct SelfTradeCancellation {
    Order aggressor_order;
    Order resting_order;
    int volume;
    int price;
};

struct MatchResult {
    std::vector<Trade> trades{};
    std::vector<SelfTradeCancellation> cancellations{};
};

struct BookSnapshot {
    std::vector<Order> bids{};
    std::vector<Order> asks{};
    std::string time{};
};

#endif //TYPES_H
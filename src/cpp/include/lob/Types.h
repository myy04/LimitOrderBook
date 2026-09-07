#ifndef TYPES_H
#define TYPES_H

#include "Config.h"

#include <string>
#include <vector>
#include <iostream>
#include <chrono>

enum class OrderSide : uint8_t {BUY, SELL};

inline std::ostream& operator<<(std::ostream& os, OrderSide side) {
    switch (side) {
        case OrderSide::BUY: return os << "BUY";
        case OrderSide::SELL: return os << "SELL";
    }
}

struct Mpid {
    uint32_t tag;

    Mpid(): tag{0} {}

    Mpid(const char* s) : tag{0} {
        for (int i = 0; i < 4; i++) {
            tag <<= 8;
            tag |= static_cast<uint32_t>(s[i]);
        }
    }

    const bool operator==(const Mpid& other) const {return tag == other.tag;}
};

inline std::ostream& operator<<(std::ostream& os, const Mpid& id) {    
    char s[5];
    s[0] = static_cast<char>((id.tag >> 24) & 0xFF); 
    s[1] = static_cast<char>((id.tag >> 16) & 0xFF);
    s[2] = static_cast<char>((id.tag >>  8) & 0xFF);
    s[3] = static_cast<char>( id.tag        & 0xFF);
    s[4] = '\0';
    return os << s;
} 

struct Order {
    uint64_t price; // 8 byte
    uint64_t volume; // 8 byte
    uint64_t order_id; // 8 byte
    uint64_t timestamp; //ns 8 byte
    Mpid trader_id; // 4 byte
    OrderSide side; // 1 byte

    using side_t = decltype(side);
    using price_t = decltype(price);
    using volume_t = decltype(volume);
    using order_id_t = decltype(order_id);
    using trader_id_t = decltype(trader_id);
    using timestamp_t = decltype(timestamp);
};

inline std::ostream& operator<<(std::ostream& os, const Order& ord) {
    return os << ord.trader_id << ' ' << ord.side << ' ' << ord.price * CONFIG::PRICE_TICK_SIZE << ' ' << ord.volume << ' ' << ord.timestamp << ' ' << ord.order_id;
}

struct Trade {
    Order::order_id_t aggressor_order_id;
    Order::order_id_t resting_order_id;
    int price;
    int volume;
};

struct SelfTradeCancellation {
    Order::order_id_t aggressor_order_id;
    Order::order_id_t resting_order_id;
    int price;
    int volume;
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
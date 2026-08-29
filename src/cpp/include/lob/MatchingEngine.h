#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "./Types.h"
#include "./OrderBook.h"
#include "./SnapshotBuffer.h"
#include "./Config.h"

#include <chrono>

class MatchingEngine {  
public: 
    explicit MatchingEngine() = default;

    MatchResult handle_order(std::shared_ptr<Order> order); 
    std::vector<MatchResult> handle_orders(std::vector<std::shared_ptr<Order>> orders);

    BookSnapshot pull_snapshot();

private:
    MatchResult handle_buy(std::shared_ptr<Order> order);
    MatchResult handle_sell(std::shared_ptr<Order> order);
    SelfTradeCancellation handle_self_trade(std::shared_ptr<Order> aggressor_order, std::shared_ptr<Order> resting_order);

    void push_snapshot(BookSnapshot);

    OrderBook order_book{};
    std::shared_ptr<SnapshotBuffer> snapshot_buffer{}; 
    decltype(std::chrono::steady_clock::now()) last_snapshot_time{};
};  

#endif
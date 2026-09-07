#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "./Types.h"
#include "./OrderBook.h"
#include "./SnapshotBuffer.h"
#include "./Config.h"

#include <chrono>

class MatchingEngine {  
public: 
    explicit MatchingEngine(); 

    MatchResult handle_order(const std::shared_ptr<Order>& order); 
    
    void reset();
    
    BookSnapshot pull_snapshot();

private:
    MatchResult handle_buy(const std::shared_ptr<Order>& order);
    MatchResult handle_sell(const std::shared_ptr<Order>& order);
    SelfTradeCancellation handle_self_trade(const std::shared_ptr<Order>& aggressor_order, const std::shared_ptr<Order>& resting_order);

    void push_snapshot(BookSnapshot);

    OrderBook order_book;
    std::shared_ptr<SnapshotBuffer> snapshot_buffer; 
    decltype(std::chrono::steady_clock::now()) last_snapshot_time;
};  

#endif
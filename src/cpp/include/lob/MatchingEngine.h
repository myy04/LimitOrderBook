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

    MatchResult handle_order(Order&& order); 
    
    void reset();
    
    BookSnapshot pull_snapshot();
    
private:
    MatchResult handle_buy(Order& order);
    MatchResult handle_sell(Order& order);
    SelfTradeCancellation handle_self_trade(Order& aggressor_order, Order& resting_order);
    
    void push_snapshot(BookSnapshot);
    
    OrderBook order_book;
    std::shared_ptr<SnapshotBuffer> snapshot_buffer; 
    decltype(std::chrono::steady_clock::now()) last_snapshot_time;
};  

#endif
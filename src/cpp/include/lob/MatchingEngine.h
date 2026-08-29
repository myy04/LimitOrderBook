#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "./Types.h"
#include "./OrderBook.h"
#include "./SnapshotBuffer.h"

class MatchingEngine {  
public: 
    explicit MatchingEngine(std::shared_ptr<SnapshotBuffer> buffer);
    MatchResult handle_order(std::shared_ptr<Order> order); 
    std::vector<MatchResult> handle_orders(std::vector<std::shared_ptr<Order>> orders);
private:
    MatchResult handle_buy(std::shared_ptr<Order> order);
    MatchResult handle_sell(std::shared_ptr<Order> order);
    void save_snapshot(int depth);
    SelfTradeCancellation handle_self_trade(std::shared_ptr<Order> aggressor_order, std::shared_ptr<Order> resting_order);

    OrderBook order_book;
    std::shared_ptr<SnapshotBuffer> snapshot_buffer; 
};  

#endif
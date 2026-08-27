#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "./Types.h"
#include "./OrderBook.h"

class MatchingEngine {  
public: 
    explicit MatchingEngine();
    MatchResult handle_order(std::shared_ptr<Order> order); 
    std::vector<MatchResult> handle_orders(std::vector<std::shared_ptr<Order>> orders);
private:
    MatchResult handle_buy(std::shared_ptr<Order> order);
    MatchResult handle_sell(std::shared_ptr<Order> order);
    void save_snapshot(int depth);
    SelfTradeCancellation handle_self_trade(std::shared_ptr<Order> aggressor_order, std::shared_ptr<Order> resting_order);

    OrderBook order_book;
};  

#endif
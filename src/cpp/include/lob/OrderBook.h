#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "./Types.h"
#include <map>
#include <unordered_map>
#include <list>
#include <memory>

class OrderBook {
public:
    OrderBook();    

    void insert_order(std::shared_ptr<Order>) noexcept;
    void remove_order(std::shared_ptr<Order>) noexcept;
    std::shared_ptr<Order> peek_best_bid();
    std::shared_ptr<Order> peek_best_ask();

private:
    std::map<int, std::list<std::shared_ptr<Order>>> bids;
    std::map<int, std::list<std::shared_ptr<Order>>> asks;
    std::unordered_map<int, std::list<std::shared_ptr<Order>>::iterator> nodes; 
};


#endif 
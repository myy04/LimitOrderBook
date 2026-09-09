#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "./Types.h"
#include "./Config.h"

#include <map>
#include <unordered_map>
#include <list>
#include <memory>
#include <chrono>
#include <ctime>
#include <vector>

class Pool {
public:
    Pool(): pool{}, pos{} {
        pool.reserve(2e6);
    }

    void insert(const Order& order) {   
        pool.push_back(order);
        auto index = pool.size() - 1;
        pos[order.order_id] = index; 
    }  

    Order& get(Order::order_id_t order_id) {
        auto index = pos.at(order_id);
        return pool[index];
    }

    void clear() {
        pool.clear();
        pos.clear();
    }

private:    
    std::vector<Order> pool;
    std::unordered_map<Order::order_id_t, size_t> pos;
};

class OrderBook {
public:
    explicit OrderBook();    

    void insert_order(const Order& order);
    void remove_order(const Order& order);
    Order& peek_best_bid();
    Order& peek_best_ask();
    BookSnapshot get_snapshot();

    bool is_ask_tree_empty();
    bool is_bid_tree_empty();

    void print_orderbook();

    void reset();

    const Order& get_order(Order::order_id_t);

private:
    std::map<int, std::list<size_t>> bid_tree; // price -> list of orders_id
    std::map<int, std::list<size_t>> ask_tree; // price -> list of orders_id
    std::unordered_map<int, std::list<size_t>::iterator> order_to_node; // order_id -> list iterator
    
    Pool pool;
};


#endif 
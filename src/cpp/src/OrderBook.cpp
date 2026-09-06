#include ".././include/lob/OrderBook.h"

#include <cassert>

OrderBook::OrderBook(): bids{}, asks{}, nodes{} {}

void OrderBook::insert_order(std::shared_ptr<Order> order) {
    auto& tree = (order->side == OrderSide::BUY) ? bids : asks;
    auto& list = tree[order->price];
    list.push_back(order);
    nodes[order->order_id] = --list.end();
}  

void OrderBook::remove_order(std::shared_ptr<Order> order) {
    if (nodes.find(order->order_id) == nodes.end()) throw "order does not exist in the orderbook";
    auto& tree = (order->side == OrderSide::BUY) ? bids : asks;
    auto& list = tree.at(order->price);
    auto& node = nodes.at(order->order_id);
    list.erase(node);
    nodes.erase(order->order_id);

    if (list.empty()) tree.erase(order->price);
}

std::shared_ptr<Order> OrderBook::peek_best_bid() {         
    if (bids.empty()) throw "no bids";
    auto list = bids.rbegin()->second; 
    return *list.begin();
}

std::shared_ptr<Order> OrderBook::peek_best_ask() { 
    if (asks.empty()) throw "no asks";
    auto list = asks.begin()->second;
    return *list.begin();
}

BookSnapshot OrderBook::get_snapshot() {
    const int depth = CONFIG::SNAPSHOT_DEPTH;

    BookSnapshot snapshot{};

    for (auto i = bids.rbegin(); i != bids.rend(); i++) {
        if (snapshot.bids.size() >= depth) break;
        
        const int price = i->first;
        const auto& list = i->second;
    
        for (auto& order : list) {
            snapshot.bids.push_back(*order);
            if (snapshot.bids.size() >= depth) break;
        }
    }

    for (auto i = asks.begin(); i != asks.end(); i++) {
        if (snapshot.asks.size() >= depth) break;
        
        const int price = i->first;
        const auto& list = i->second;
    
        for (auto& order : list) {
            snapshot.asks.push_back(*order);
            if (snapshot.asks.size() >= depth) break;
        }
    }

    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local_time);
    std::string time_str(buffer);
    
    snapshot.time = time_str;

    return snapshot;
}
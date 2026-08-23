#include ".././include/lob/OrderBook.h"

OrderBook::OrderBook(): bids{}, asks{}, nodes{} {}

void OrderBook::insert_order(std::shared_ptr<Order> order) noexcept {
    auto& tree = (order->side == OrderSide::BUY) ? bids : asks;
    if (tree.find(order->price) == tree.end()) tree[order->price] = {};
    auto& list = tree[order->price];
    list.push_back(order);
    nodes[order->order_id] = --list.end();
}  

void OrderBook::remove_order(std::shared_ptr<Order> order) noexcept {
    if (nodes.find(order->order_id) == nodes.end()) return;
    auto& tree = (order->side == OrderSide::BUY) ? bids : asks;
    auto& list = tree[order->price];
    auto& node = nodes[order->order_id];
    list.erase(node);
    nodes.erase(order->order_id);

    if (list.empty()) tree.erase(order->price);
}

std::shared_ptr<Order> OrderBook::peek_best_bid() {         
    if (bids.empty()) throw "No bids are in the orderbook";
    auto& list = bids.rbegin()->second;
    return *list.begin();
}

std::shared_ptr<Order> OrderBook::peek_best_ask() { 
    if (asks.empty()) throw "no asks are in the orderbook";
    auto& list = asks.begin()->second;
    return *list.begin();
}
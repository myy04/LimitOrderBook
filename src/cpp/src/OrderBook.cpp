#include ".././include/lob/OrderBook.h"

#include <cassert>

OrderBook::OrderBook(): pool{}, ask_tree{}, bid_tree{}, order_to_node{} {}

void OrderBook::insert_order(const Order& order) {
    pool.insert(order);
    
    auto& tree = (order.side == OrderSide::BUY) ? bid_tree : ask_tree;
    auto& list = tree[order.price];
    list.push_back(order.order_id);
    order_to_node[order.order_id] = --list.end();
}  

void OrderBook::remove_order(const Order& order) {
    auto& tree = (order.side == OrderSide::BUY) ? bid_tree : ask_tree;
    auto& list = tree.at(order.price);
    auto& node = order_to_node.at(order.order_id);
    list.erase(node);
    order_to_node.erase(order_to_node.find(order.order_id));
    if (list.empty()) tree.erase(order.price);
}

Order& OrderBook::peek_best_bid() {         
    if (bid_tree.empty()) throw "no bids";
    auto list = bid_tree.rbegin()->second; 
    return pool.get(*list.begin());
}

Order& OrderBook::peek_best_ask() { 
    if (ask_tree.empty()) throw "no asks";
    auto list = ask_tree.begin()->second;
    return pool.get(*list.begin());
}

BookSnapshot OrderBook::get_snapshot() {
    const int depth = CONFIG::SNAPSHOT_DEPTH;

    BookSnapshot snapshot{};

    for (auto i = bid_tree.rbegin(); i != bid_tree.rend(); i++) {
        if (snapshot.bids.size() >= depth) break;
        const auto& [price, list] = *i;
    
        for (auto& order_id : list) {
            snapshot.bids.push_back(pool.get(order_id));
            if (snapshot.bids.size() >= depth) break;
        }
    }

    for (const auto& [price, list] : ask_tree) {
        if (snapshot.asks.size() >= depth) break;

        for (auto& order_id : list) {
            snapshot.asks.push_back(pool.get(order_id));
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


void OrderBook::print_orderbook() {
    std::cout << "Bids:\n";
    for (const auto& [price, bid_list] : bid_tree) {
        std::cout << "PRICE: " << price << "\n";
        for (const auto& order_id : bid_list) {
            std::cout << pool.get(order_id) << '\n';
        }
    }

    std::cout << "Asks:\n";
    for (const auto& [price, ask_list] : ask_tree) {
        std::cout << "PRICE: " << price << "\n";
        for (const auto& order_id : ask_list) {
            std::cout << pool.get(order_id) << '\n';
        }
    }
}
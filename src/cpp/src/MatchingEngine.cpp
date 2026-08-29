#include ".././include/lob/MatchingEngine.h"

MatchingEngine::MatchingEngine(std::shared_ptr<SnapshotBuffer> snapshot_buffer): 
                                order_book{}, snapshot_buffer{snapshot_buffer} {}

MatchResult MatchingEngine::handle_order(std::shared_ptr<Order> order) {
    if (order->side == OrderSide::BUY) return handle_buy(std::move(order));
    else return handle_sell(std::move(order));
}

std::vector<MatchResult> MatchingEngine::handle_orders(std::vector<std::shared_ptr<Order>> orders) {
    std::vector<MatchResult> results{orders.size(), MatchResult{}};
    for (int order_idx = 0; order_idx < orders.size(); order_idx++) {
        auto& order = orders[order_idx];
        results[order_idx] = handle_order(order); 
    }
    return results;
}

MatchResult MatchingEngine::handle_buy(std::shared_ptr<Order> order) {
    MatchResult ret{};

    while (order->volume > 0) {
        auto best_ask = order_book.peek_best_ask();
        if (best_ask == nullptr || best_ask->price > order->price) break;
        
        if (best_ask->trader_id == order->trader_id) {
            ret.cancellations.emplace_back(handle_self_trade(order, best_ask));
            continue;  
        }

        Trade trade{};
        trade.volume = std::min(order->volume, best_ask->volume);
        trade.price = best_ask->price;
        trade.aggressor_order = *order;
        trade.resting_order = *best_ask;

        order->volume -= trade.volume;
        best_ask->volume -= trade.volume;   
        if (best_ask->volume == 0) order_book.remove_order(best_ask);

        ret.trades.emplace_back(std::move(trade));
    }

    if (order->volume > 0) order_book.insert_order(std::move(order));
    return ret;
}


MatchResult MatchingEngine::handle_sell(std::shared_ptr<Order> order) {
    MatchResult ret{};

    while (order->volume > 0) {
        auto best_bid = order_book.peek_best_bid();
        if (best_bid == nullptr || best_bid->price < order->price) break;
        
        if (best_bid->trader_id == order->trader_id) {
            ret.cancellations.emplace_back(handle_self_trade(order, best_bid));
            continue;  
        }

        Trade trade{};
        trade.volume = std::min(order->volume, best_bid->volume);
        trade.price = best_bid->price;
        trade.aggressor_order = *order;
        trade.resting_order = *best_bid;

        order->volume -= trade.volume;
        best_bid->volume -= trade.volume;   
        if (best_bid->volume == 0) order_book.remove_order(best_bid);

        ret.trades.emplace_back(std::move(trade));
    }

    if (order->volume > 0) order_book.insert_order(std::move(order));
    return ret;
}


SelfTradeCancellation MatchingEngine::handle_self_trade(std::shared_ptr<Order> aggressor_order, std::shared_ptr<Order> resting_order) {
    SelfTradeCancellation cancel{};
    cancel.volume = std::min(aggressor_order->volume, resting_order->volume);
    cancel.price = resting_order->price;
    cancel.resting_order = *resting_order;
    cancel.aggressor_order = *aggressor_order;
    resting_order->volume -= cancel.volume;
    aggressor_order->volume -= cancel.volume;
    if (resting_order->volume == 0) order_book.remove_order(resting_order);
    return cancel;
}


void MatchingEngine::save_snapshot(int depth) {
    BookSnapshot snapshot = order_book.get_snapshot(depth);
    snapshot_buffer->push(snapshot);
}

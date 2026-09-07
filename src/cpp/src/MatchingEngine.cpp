#include ".././include/lob/MatchingEngine.h"

MatchingEngine::MatchingEngine(): order_book{}, snapshot_buffer{std::make_shared<SnapshotBuffer>()}, last_snapshot_time{} {}

MatchResult MatchingEngine::handle_order(Order&& order) {
    auto result = (order.side == OrderSide::BUY) ? handle_buy(order) : handle_sell(order);

    if constexpr (CONFIG::CAPTURE_SNAPSHOTS) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_snapshot_time) > CONFIG::SNAPSHOT_PERIOD) {
            push_snapshot(std::move(order_book.get_snapshot()));
            last_snapshot_time = now;
        }
    }
    
    if constexpr (CONFIG::DEBUG_OUTPUT) {
        std::cout << '\n';
        for (int i = 1; i <= 100; i++) std::cout << '-';
        std::cout << '\n';
        order_book.print_orderbook();
        std::cout << '\n';
        for (int i = 1; i <= 100; i++) std::cout << '-';
        std::cout << '\n';
    }

    return result;
}

MatchResult MatchingEngine::handle_buy(Order& order) {
    MatchResult ret{};

    try {
        while (order.volume > 0) {
            auto best_ask = order_book.peek_best_ask();
            if (best_ask.price > order.price) break;
            
            if (best_ask.trader_id == order.trader_id) {
                ret.cancellations.emplace_back(handle_self_trade(order, best_ask));
                continue;  
            }

            Trade trade{};
            trade.volume = std::min(order.volume, best_ask.volume);
            trade.price = best_ask.price;
            trade.aggressor_order_id = order.order_id;
            trade.resting_order_id = best_ask.order_id;

            order.volume -= trade.volume;
            best_ask.volume -= trade.volume;   
            if (best_ask.volume == 0) order_book.remove_order(best_ask);

            ret.trades.emplace_back(std::move(trade));
        }
    } catch (...) {}

    if (order.volume > 0) order_book.insert_order(std::move(order));
    return ret;
}


MatchResult MatchingEngine::handle_sell(Order& order) {
    MatchResult ret{};

    try {
        while (order.volume > 0) {
            auto best_bid = order_book.peek_best_bid();
            
            if (best_bid.price < order.price) break;
            
            if (best_bid.trader_id == order.trader_id) {
                ret.cancellations.emplace_back(handle_self_trade(order, best_bid));
                continue;  
            }

            Trade trade{};
            trade.volume = std::min(order.volume, best_bid.volume);
            trade.price = best_bid.price;
            trade.aggressor_order_id = order.order_id;
            trade.resting_order_id = best_bid.order_id;

            order.volume -= trade.volume;
            best_bid.volume -= trade.volume;   
            if (best_bid.volume == 0) order_book.remove_order(best_bid);

            ret.trades.emplace_back(std::move(trade));
        }
    } catch (...) {}

    if (order.volume > 0) order_book.insert_order(std::move(order));
    return ret;
}


SelfTradeCancellation MatchingEngine::handle_self_trade(Order& aggressor_order, Order& resting_order) {
    SelfTradeCancellation cancel{};
    cancel.volume = std::min(aggressor_order.volume, resting_order.volume);
    cancel.price = resting_order.price;
    cancel.resting_order_id = resting_order.order_id;
    cancel.aggressor_order_id = aggressor_order.order_id;
    resting_order.volume -= cancel.volume;
    aggressor_order.volume -= cancel.volume;
    if (resting_order.volume == 0) order_book.remove_order(resting_order);
    return cancel;
}


void MatchingEngine::push_snapshot(BookSnapshot snapshot) { 
    if (snapshot_buffer == nullptr) throw std::runtime_error("snapshot buffer is not init");
    snapshot_buffer->push(std::move(snapshot));
} 

BookSnapshot MatchingEngine::pull_snapshot() {
    if (snapshot_buffer == nullptr) throw std::runtime_error("snapshot buffer is not init");
    return snapshot_buffer->pull();
}

void MatchingEngine::reset() {
    order_book = OrderBook();
}
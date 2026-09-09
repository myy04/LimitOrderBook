#include "../include/lob/OrderGateway.h"

MatchResult OrderGateway::submit_order(OrderRequest ord) {
    if (!is_order_valid(ord)) throw GatewayException("Order request is invalid");
    return engine.handle_order(create_order(ord));
}

Order OrderGateway::create_order(const OrderRequest& order_request) {
    Order::price_t converted_price = convert_price(order_request.price);
    
    Order order{};
    order.side = order_request.side;
    order.price = converted_price;
    order.volume = order_request.volume;
    order.order_id = ++order_counter;
    order.trader_id = order_request.trader_id;
    order.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    return order; 
}

Order::price_t OrderGateway::convert_price(decltype(OrderGateway::OrderRequest::price) orig_price) {
    if (orig_price < CONFIG::MIN_PRICE || orig_price > CONFIG::MAX_PRICE) throw GatewayException("Price is out of range");  
    decltype(OrderGateway::OrderRequest::price) nearest_tick = std::round(orig_price / CONFIG::PRICE_TICK_SIZE) * CONFIG::PRICE_TICK_SIZE;
    if (std::abs(orig_price - nearest_tick) > CONFIG::EPS) throw GatewayException("Price is not divisible by tick size");
    return std::round(orig_price / CONFIG::PRICE_TICK_SIZE);
}

bool OrderGateway::is_order_valid(OrderRequest req) {
    try {
        convert_price(req.price);
    } catch (const GatewayException&) {
        return false;
    }
    return req.volume >= CONFIG::MIN_VOLUME && req.volume <= CONFIG::MAX_VOLUME;
}

const Order& OrderGateway::get_order(Order::order_id_t order_id) {
    return engine.get_order(order_id);
}

void OrderGateway::reset() {
    engine.reset();
    order_counter = 0;
}

BookSnapshot OrderGateway::pull_snapshot() {
    return engine.pull_snapshot();
}

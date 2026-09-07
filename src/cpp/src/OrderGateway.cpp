#include "../include/lob/OrderGateway.h"

MatchResult OrderGateway::submit_order(OrderRequest ord) {
    return engine->handle_order(create_order(ord));
}

std::shared_ptr<Order> OrderGateway::create_order(const OrderRequest& order_request) {
    if (order_request.side == OrderSide::UNDEFINED) throw GatewayException("Order side is undefined");
    Order::price_t converted_price = convert_price(order_request.price);
    
    Order order{};
    order.price = converted_price;
    order.volume = order_request.volume;
    order.order_id = ++order_counter;
    order.trader_id = order_request.trader_id;
    order.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    return std::make_shared<Order>(std::move(order)); 
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
        if (req.volume < CONFIG::MIN_VOLUME || req.volume > CONFIG::MAX_VOLUME) return false;
        return true;
    } catch (const GatewayException&) {
        return false;
    }
}
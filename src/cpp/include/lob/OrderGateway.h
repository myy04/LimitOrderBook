#ifndef ORDER_GATEWAY_H
#define ORDER_GATEWAY_H

#include "Config.h"
#include "Types.h"
#include "MatchingEngine.h"

#include <exception>
#include <cmath>
#include <chrono>

class OrderGateway {
public:
    struct OrderRequest {
        OrderSide side;
        float price;
        size_t volume;
        Mpid trader_id;
    };

    explicit OrderGateway(): engine{} {}

    MatchResult submit_order(OrderRequest order);
    static bool is_order_valid(OrderRequest req);

    const Order& get_order(Order::order_id_t);

    void reset();

    BookSnapshot pull_snapshot();

private:    
    Order create_order(const OrderRequest& order_request);
    MatchingEngine engine;

    static Order::price_t convert_price(decltype(OrderGateway::OrderRequest::price));

    size_t order_counter = 0; 
};

class GatewayException : public std::runtime_error {
public:
    GatewayException(const std::string& msg): std::runtime_error(msg) {}
};

#endif
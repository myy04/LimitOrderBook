#ifndef ORDER_GENERATOR_H
#define ORDER_GENERATOR_H

#include "Config.h"
#include "Types.h"
#include "OrderGateway.h"

#include <array>
#include <random>
#include <cmath>

namespace {
    using price_t = decltype(OrderGateway::OrderRequest::price);
    using volume_t = decltype(OrderGateway::OrderRequest::volume);
    using side_t = decltype(OrderGateway::OrderRequest::side);
    using mpid_t = decltype(OrderGateway::OrderRequest::trader_id);

    std::array<mpid_t, 8> MP_IDENTIFIERS = {
        "CDEL",
        "GTSC",
        "JETA",
        "NITE",
        "VIRT",
        "GSCO",
        "UBSS",
        "JPMS"
    };

    size_t MP_IDENTIFIERS_NUM = MP_IDENTIFIERS.size();
}

class OrderGenerator {
public:
    explicit OrderGenerator(const int seed);
    OrderGateway::OrderRequest generate_order();
private:  

    std::mt19937 gen;
    std::normal_distribution<float> price_dist;
    std::normal_distribution<float> volume_dist;
    std::uniform_int_distribution<size_t> side_dist; 
    std::uniform_int_distribution<size_t> mpid_dist;

    price_t generate_price();
    volume_t generate_volume();
    side_t generate_side();
    mpid_t generate_mpid();

    inline const static price_t price_sd = 10;   
    inline const static volume_t volume_sd = 10;

    price_t last_price;
    volume_t last_volume;
};

#endif
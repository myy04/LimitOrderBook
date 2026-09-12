#include "../include/lob/OrderGenerator.h"

OrderGenerator::OrderGenerator(int seed): gen(seed), 
                                    price_dist(1000, price_sd), 
                                    volume_dist(1000, volume_sd),
                                    side_dist(0, 1),
                                    mpid_dist(size_t(0), (MP_IDENTIFIERS_NUM) - 1)
{
    last_price = generate_price();
    last_volume = generate_volume();
}

OrderRequest OrderGenerator::generate_order() {
    OrderRequest ord;
    while (true) {
        ord.price = generate_price();
        ord.volume = generate_volume();
        ord.side = generate_side();
        ord.trader_id = generate_mpid();
        if (OrderGateway::is_order_valid(ord)) break;
    }
    return ord;
}

price_t OrderGenerator::generate_price() {
    last_price = std::round(price_dist(gen) / CONFIG::PRICE_TICK_SIZE) * CONFIG::PRICE_TICK_SIZE;
    price_dist = std::normal_distribution<float>(last_price, price_sd);
    return last_price;
}

volume_t OrderGenerator::generate_volume() {
    last_volume = std::round(volume_dist(gen));
    volume_dist = std::normal_distribution<float>(last_volume, volume_sd);
    return last_volume;
}

side_t OrderGenerator::generate_side() {
    return side_dist(gen) ? OrderSide::BUY : OrderSide::SELL;
}

mpid_t OrderGenerator::generate_mpid() {
    return MP_IDENTIFIERS[mpid_dist(gen)];
}

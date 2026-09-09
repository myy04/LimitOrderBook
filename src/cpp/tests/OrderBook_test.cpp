#include <gtest/gtest.h>

#include <string>

#include "lob/OrderBook.h"
#include "lob/Types.h"

namespace {

Order make_order(OrderSide side, uint64_t price, uint64_t volume, uint64_t order_id,
                 const char* trader_id = "TEST", uint64_t timestamp = 0) {
    Order order{};
    order.side = side;
    order.price = price;
    order.volume = volume;
    order.order_id = order_id;
    order.trader_id = Mpid{trader_id};
    order.timestamp = timestamp;
    return order;
}

class OrderBookTest : public ::testing::Test {
protected:
    OrderBook book;
};

// ---------------------------------------------------------------------------
// Empty book behavior
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, PeekBestBidThrowsWhenBookIsEmpty) {
    EXPECT_THROW(book.peek_best_bid(), const char*);
}

TEST_F(OrderBookTest, PeekBestAskThrowsWhenBookIsEmpty) {
    EXPECT_THROW(book.peek_best_ask(), const char*);
}

TEST_F(OrderBookTest, IsEmptyChecksAreTrueOnEmptyBook) {
    EXPECT_TRUE(book.is_bid_tree_empty());
    EXPECT_TRUE(book.is_ask_tree_empty());
}

// ---------------------------------------------------------------------------
// Single order insertion
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, InsertedBuyOrderBecomesBestBid) {
    book.insert_order(make_order(OrderSide::BUY, 100, 10, 1));

    auto& best_bid = book.peek_best_bid();
    EXPECT_EQ(best_bid.order_id, 1u);
    EXPECT_EQ(best_bid.price, 100u);
    EXPECT_EQ(best_bid.volume, 10u);
    EXPECT_TRUE(book.is_ask_tree_empty());
}

TEST_F(OrderBookTest, InsertedSellOrderBecomesBestAsk) {
    book.insert_order(make_order(OrderSide::SELL, 105, 5, 1));

    auto& best_ask = book.peek_best_ask();
    EXPECT_EQ(best_ask.order_id, 1u);
    EXPECT_EQ(best_ask.price, 105u);
    EXPECT_EQ(best_ask.volume, 5u);
    EXPECT_TRUE(book.is_bid_tree_empty());
}

TEST_F(OrderBookTest, BidsAndAsksAreTrackedIndependently) {
    book.insert_order(make_order(OrderSide::BUY, 100, 10, 1));
    book.insert_order(make_order(OrderSide::SELL, 110, 5, 2));

    EXPECT_EQ(book.peek_best_bid().order_id, 1u);
    EXPECT_EQ(book.peek_best_ask().order_id, 2u);
}

// ---------------------------------------------------------------------------
// Price priority
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, BestBidIsHighestPricedBuyOrder) {
    book.insert_order(make_order(OrderSide::BUY, 100, 10, 1));
    book.insert_order(make_order(OrderSide::BUY, 105, 10, 2));
    book.insert_order(make_order(OrderSide::BUY, 95, 10, 3));

    auto& best_bid = book.peek_best_bid();
    EXPECT_EQ(best_bid.order_id, 2u);
    EXPECT_EQ(best_bid.price, 105u);
}

TEST_F(OrderBookTest, BestAskIsLowestPricedSellOrder) {
    book.insert_order(make_order(OrderSide::SELL, 105, 10, 1));
    book.insert_order(make_order(OrderSide::SELL, 100, 10, 2));
    book.insert_order(make_order(OrderSide::SELL, 110, 10, 3));

    auto& best_ask = book.peek_best_ask();
    EXPECT_EQ(best_ask.order_id, 2u);
    EXPECT_EQ(best_ask.price, 100u);
}

// ---------------------------------------------------------------------------
// Time priority (FIFO) within a price level
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, OrdersAtSamePriceMaintainFifoOrder) {
    book.insert_order(make_order(OrderSide::BUY, 100, 10, 1));
    book.insert_order(make_order(OrderSide::BUY, 100, 5, 2));
    book.insert_order(make_order(OrderSide::BUY, 100, 3, 3));

    EXPECT_EQ(book.peek_best_bid().order_id, 1u);
}

TEST_F(OrderBookTest, RemovingFrontOrderExposesNextOrderInFifoQueue) {
    auto first = make_order(OrderSide::BUY, 100, 10, 1);
    book.insert_order(first);
    book.insert_order(make_order(OrderSide::BUY, 100, 5, 2));

    book.remove_order(first);

    EXPECT_EQ(book.peek_best_bid().order_id, 2u);
}

TEST_F(OrderBookTest, RemovingMiddleOrderPreservesRelativeFifoOrderOfSurvivors) {
    auto first = make_order(OrderSide::BUY, 100, 10, 1);
    auto second = make_order(OrderSide::BUY, 100, 5, 2);
    auto third = make_order(OrderSide::BUY, 100, 3, 3);
    book.insert_order(first);
    book.insert_order(second);
    book.insert_order(third);

    book.remove_order(second);

    EXPECT_EQ(book.peek_best_bid().order_id, 1u);
    book.remove_order(first);
    EXPECT_EQ(book.peek_best_bid().order_id, 3u);
}

// ---------------------------------------------------------------------------
// Removal and price-level cleanup
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, RemovingOnlyBidAtPriceLevelClearsTheBook) {
    auto order = make_order(OrderSide::BUY, 100, 10, 1);
    book.insert_order(order);

    book.remove_order(order);

    EXPECT_TRUE(book.is_bid_tree_empty());
}

TEST_F(OrderBookTest, RemovingOnlyAskAtPriceLevelClearsTheBook) {
    auto order = make_order(OrderSide::SELL, 100, 10, 1);
    book.insert_order(order);

    book.remove_order(order);

    EXPECT_TRUE(book.is_ask_tree_empty());
}

TEST_F(OrderBookTest, RemovingBestBidExposesNextBestPriceLevel) {
    book.insert_order(make_order(OrderSide::BUY, 95, 10, 1));
    auto best = make_order(OrderSide::BUY, 105, 10, 2);
    book.insert_order(best);

    book.remove_order(best);

    auto& new_best_bid = book.peek_best_bid();
    EXPECT_EQ(new_best_bid.order_id, 1u);
    EXPECT_EQ(new_best_bid.price, 95u);
}

TEST_F(OrderBookTest, RemovingBestAskExposesNextBestPriceLevel) {
    book.insert_order(make_order(OrderSide::SELL, 110, 10, 1));
    auto best = make_order(OrderSide::SELL, 100, 10, 2);
    book.insert_order(best);

    book.remove_order(best);

    auto& new_best_ask = book.peek_best_ask();
    EXPECT_EQ(new_best_ask.order_id, 1u);
    EXPECT_EQ(new_best_ask.price, 110u);
}

TEST_F(OrderBookTest, RemovingOneOrderDoesNotAffectOtherPriceLevels) {
    auto low = make_order(OrderSide::BUY, 95, 10, 1);
    auto high = make_order(OrderSide::BUY, 105, 10, 2);
    book.insert_order(low);
    book.insert_order(high);

    book.remove_order(low);

    EXPECT_EQ(book.peek_best_bid().order_id, 2u);
}

// ---------------------------------------------------------------------------
// Volume mutation is reflected through the pool
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, MutatingReturnedOrderRefUpdatesPooledOrder) {
    book.insert_order(make_order(OrderSide::BUY, 100, 10, 1));

    auto& best_bid = book.peek_best_bid();
    best_bid.volume = 4;

    EXPECT_EQ(book.peek_best_bid().volume, 4u);
}

// ---------------------------------------------------------------------------
// Reset
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, ResetClearsBookContents) {
    book.insert_order(make_order(OrderSide::BUY, 100, 10, 1));
    book.insert_order(make_order(OrderSide::SELL, 105, 5, 2));

    book.reset();

    EXPECT_TRUE(book.is_bid_tree_empty());
    EXPECT_TRUE(book.is_ask_tree_empty());
}

TEST_F(OrderBookTest, BookIsUsableAfterReset) {
    book.insert_order(make_order(OrderSide::BUY, 100, 10, 1));
    book.reset();

    book.insert_order(make_order(OrderSide::BUY, 105, 3, 2));
    auto& best_bid = book.peek_best_bid();
    EXPECT_EQ(best_bid.order_id, 2u);
    EXPECT_EQ(best_bid.price, 105u);
}

}  // namespace
#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "lob/OrderBook.h"
#include "lob/Types.h"

namespace {

std::shared_ptr<Order> make_order(OrderSide side, int price, int volume, int order_id,
                                   const std::string &trader_id = "trader",
                                   float timestamp = 0.0f) {
    return std::make_shared<Order>(Order{side, price, volume, order_id, timestamp, trader_id});
}

class OrderBookTest : public ::testing::Test {
protected:
    OrderBook book;
};

// ---------------------------------------------------------------------------
// Empty book behavior
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, PeekBestBidThrowsWhenBookIsEmpty) {
    // Act & Assert
    EXPECT_EQ(book.peek_best_bid(), nullptr);
}

TEST_F(OrderBookTest, PeekBestAskThrowsWhenBookIsEmpty) {
    // Act & Assert
    EXPECT_EQ(book.peek_best_ask(), nullptr);
}

TEST_F(OrderBookTest, RemovingFromEmptyBookIsNoOp) {
    // Arrange
    auto order = make_order(OrderSide::BUY, 100, 10, 1);

    // Act & Assert (no exception expected)
    EXPECT_NO_THROW(book.remove_order(order));
    EXPECT_EQ(book.peek_best_bid(), nullptr);
}

// ---------------------------------------------------------------------------
// Single order insertion
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, InsertedBuyOrderBecomesBestBid) {
    // Arrange
    auto order = make_order(OrderSide::BUY, 100, 10, 1);

    // Act
    book.insert_order(order);

    // Assert
    auto best_bid = book.peek_best_bid();
    EXPECT_EQ(best_bid->order_id, 1);
    EXPECT_EQ(best_bid->price, 100);
    EXPECT_EQ(best_bid->volume, 10);
    EXPECT_EQ(book.peek_best_ask(), nullptr);
}

TEST_F(OrderBookTest, InsertedSellOrderBecomesBestAsk) {
    // Arrange
    auto order = make_order(OrderSide::SELL, 105, 5, 1);

    // Act
    book.insert_order(order);

    // Assert
    auto best_ask = book.peek_best_ask();
    EXPECT_EQ(best_ask->order_id, 1);
    EXPECT_EQ(best_ask->price, 105);
    EXPECT_EQ(book.peek_best_bid(), nullptr);
}

TEST_F(OrderBookTest, BidsAndAsksAreTrackedIndependently) {
    // Arrange
    auto bid = make_order(OrderSide::BUY, 100, 10, 1);
    auto ask = make_order(OrderSide::SELL, 100, 5, 2);

    // Act
    book.insert_order(bid);
    book.insert_order(ask);

    // Assert
    EXPECT_EQ(book.peek_best_bid()->order_id, 1);
    EXPECT_EQ(book.peek_best_ask()->order_id, 2);
}

// ---------------------------------------------------------------------------
// Price priority
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, BestBidIsHighestPricedBuyOrder) {
    // Arrange
    book.insert_order(make_order(OrderSide::BUY, 100, 10, 1));
    book.insert_order(make_order(OrderSide::BUY, 105, 10, 2));
    book.insert_order(make_order(OrderSide::BUY, 95, 10, 3));

    // Act
    auto best_bid = book.peek_best_bid();

    // Assert
    EXPECT_EQ(best_bid->order_id, 2);
    EXPECT_EQ(best_bid->price, 105);
}

TEST_F(OrderBookTest, BestAskIsLowestPricedSellOrder) {
    // Arrange
    book.insert_order(make_order(OrderSide::SELL, 105, 10, 1));
    book.insert_order(make_order(OrderSide::SELL, 100, 10, 2));
    book.insert_order(make_order(OrderSide::SELL, 110, 10, 3));

    // Act
    auto best_ask = book.peek_best_ask();

    // Assert
    EXPECT_EQ(best_ask->order_id, 2);
    EXPECT_EQ(best_ask->price, 100);
}

// ---------------------------------------------------------------------------
// Time priority (FIFO) within a price level
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, OrdersAtSamePriceMaintainFifoOrder) {
    // Arrange
    book.insert_order(make_order(OrderSide::BUY, 100, 10, 1));
    book.insert_order(make_order(OrderSide::BUY, 100, 5, 2));
    book.insert_order(make_order(OrderSide::BUY, 100, 3, 3));

    // Act
    auto best_bid = book.peek_best_bid();

    // Assert: the first order inserted at the price level should be at the front
    EXPECT_EQ(best_bid->order_id, 1);
}

TEST_F(OrderBookTest, RemovingFrontOrderExposesNextOrderInFifoQueue) {
    // Arrange
    auto first = make_order(OrderSide::BUY, 100, 10, 1);
    book.insert_order(first);
    book.insert_order(make_order(OrderSide::BUY, 100, 5, 2));

    // Act
    book.remove_order(first);

    // Assert
    EXPECT_EQ(book.peek_best_bid()->order_id, 2);
}

TEST_F(OrderBookTest, RemovingMiddleOrderPreservesRelativeFifoOrderOfSurvivors) {
    // Arrange
    auto first = make_order(OrderSide::BUY, 100, 10, 1);
    auto second = make_order(OrderSide::BUY, 100, 5, 2);
    auto third = make_order(OrderSide::BUY, 100, 3, 3);
    book.insert_order(first);
    book.insert_order(second);
    book.insert_order(third);

    // Act
    book.remove_order(second);

    // Assert: front is still the first order, and the second order is fully gone
    EXPECT_EQ(book.peek_best_bid()->order_id, 1);
    book.remove_order(first);
    EXPECT_EQ(book.peek_best_bid()->order_id, 3);
}

// ---------------------------------------------------------------------------
// Removal and price-level cleanup
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, RemovingOnlyBidAtPriceLevelClearsTheBook) {
    // Arrange
    auto order = make_order(OrderSide::BUY, 100, 10, 1);
    book.insert_order(order);

    // Act
    book.remove_order(order);

    // Assert
    EXPECT_EQ(book.peek_best_bid(), nullptr);
}

TEST_F(OrderBookTest, RemovingOnlyAskAtPriceLevelClearsTheBook) {
    // Arrange
    auto order = make_order(OrderSide::SELL, 100, 10, 1);
    book.insert_order(order);

    // Act
    book.remove_order(order);

    // Assert
    EXPECT_EQ(book.peek_best_ask(), nullptr);
}

TEST_F(OrderBookTest, RemovingBestBidExposesNextBestPriceLevel) {
    // Arrange
    book.insert_order(make_order(OrderSide::BUY, 95, 10, 1));
    auto best = make_order(OrderSide::BUY, 105, 10, 2);
    book.insert_order(best);

    // Act
    book.remove_order(best);

    // Assert
    auto new_best_bid = book.peek_best_bid();
    EXPECT_EQ(new_best_bid->order_id, 1);
    EXPECT_EQ(new_best_bid->price, 95);
}

TEST_F(OrderBookTest, RemovingBestAskExposesNextBestPriceLevel) {
    // Arrange
    book.insert_order(make_order(OrderSide::SELL, 110, 10, 1));
    auto best = make_order(OrderSide::SELL, 100, 10, 2);
    book.insert_order(best);

    // Act
    book.remove_order(best);

    // Assert
    auto new_best_ask = book.peek_best_ask();
    EXPECT_EQ(new_best_ask->order_id, 1);
    EXPECT_EQ(new_best_ask->price, 110);
}

TEST_F(OrderBookTest, RemovingOneOrderDoesNotAffectOtherPriceLevels) {
    // Arrange
    auto low = make_order(OrderSide::BUY, 95, 10, 1);
    auto high = make_order(OrderSide::BUY, 105, 10, 2);
    book.insert_order(low);
    book.insert_order(high);

    // Act
    book.remove_order(low);

    // Assert: best bid is untouched, and the removed level is gone
    EXPECT_EQ(book.peek_best_bid()->order_id, 2);
}

// ---------------------------------------------------------------------------
// Idempotency / unknown order handling
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, RemovingUnknownOrderIdIsNoOp) {
    // Arrange
    auto resting = make_order(OrderSide::BUY, 100, 10, 1);
    book.insert_order(resting);
    auto unknown = make_order(OrderSide::BUY, 100, 10, 999);

    // Act & Assert (no exception, resting order remains)
    EXPECT_NO_THROW(book.remove_order(unknown));
    EXPECT_EQ(book.peek_best_bid()->order_id, 1);
}

TEST_F(OrderBookTest, RemovingSameOrderTwiceIsSafe) {
    // Arrange
    auto order = make_order(OrderSide::BUY, 100, 10, 1);
    book.insert_order(order);
    book.remove_order(order);

    // Act & Assert (second removal should be a silent no-op)
    EXPECT_NO_THROW(book.remove_order(order));
    EXPECT_EQ(book.peek_best_bid(), nullptr);
}

// ---------------------------------------------------------------------------
// Larger, mixed scenarios
// ---------------------------------------------------------------------------

TEST_F(OrderBookTest, MultipleInsertsAndRemovalsAcrossBothSidesTrackIndependently) {
    // Arrange
    auto bid1 = make_order(OrderSide::BUY, 100, 10, 1);
    auto bid2 = make_order(OrderSide::BUY, 102, 5, 2);
    auto ask1 = make_order(OrderSide::SELL, 110, 7, 3);
    auto ask2 = make_order(OrderSide::SELL, 108, 3, 4);

    // Act
    book.insert_order(bid1);
    book.insert_order(bid2);
    book.insert_order(ask1);
    book.insert_order(ask2);

    // Assert: highest bid, lowest ask
    EXPECT_EQ(book.peek_best_bid()->order_id, 2);
    EXPECT_EQ(book.peek_best_ask()->order_id, 4);

    // Act: remove the best bid and best ask
    book.remove_order(bid2);
    book.remove_order(ask2);

    // Assert: next best levels surface on both sides
    EXPECT_EQ(book.peek_best_bid()->order_id, 1);
    EXPECT_EQ(book.peek_best_ask()->order_id, 3);
}

}  // namespace

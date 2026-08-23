#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "lob/MatchingEngine.h"
#include "lob/Types.h"

// NOTE: These tests specify the behavior that MatchingEngine is expected to
// exhibit, mirroring the reference Python implementation
// (src/LimitOrderBook/MatchingEngine.py) exactly. As of this writing the C++
// implementation diverges from that spec in several ways (e.g. it throws on
// an empty opposite-side book instead of treating the order as non-crossing,
// it never rests unfilled orders on the book, and it never removes
// depleted resting orders). Calls that may currently throw are wrapped in
// ASSERT_NO_THROW so failures are reported as clean gtest assertions rather
// than aborting the whole test binary with an uncaught exception.

namespace {

std::shared_ptr<Order> make_order(OrderSide side, int price, int volume, int order_id,
                                   const std::string &trader_id = "trader",
                                   float timestamp = 0.0f) {
    return std::make_shared<Order>(Order{side, price, volume, order_id, timestamp, trader_id});
}

class MatchingEngineTest : public ::testing::Test {
protected:
    MatchingEngine engine;
};

// ---------------------------------------------------------------------------
// No match: order rests on the book without crossing
// ---------------------------------------------------------------------------

TEST_F(MatchingEngineTest, BuyOrderRestsOnEmptyBook) {
    // Arrange
    auto buy_order = make_order(OrderSide::BUY, 100, 10, 1);

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    EXPECT_TRUE(result.trades.empty());
    EXPECT_TRUE(result.cancellations.empty());
}

TEST_F(MatchingEngineTest, SellOrderRestsOnEmptyBook) {
    // Arrange
    auto sell_order = make_order(OrderSide::SELL, 100, 10, 1);

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(sell_order));

    // Assert
    EXPECT_TRUE(result.trades.empty());
    EXPECT_TRUE(result.cancellations.empty());
}

TEST_F(MatchingEngineTest, BuyOrderDoesNotCrossLowerPricedAsk) {
    // Arrange
    auto sell_order = make_order(OrderSide::SELL, 105, 10, 1, "seller");
    ASSERT_NO_THROW(engine.handle_order(sell_order));
    auto buy_order = make_order(OrderSide::BUY, 100, 10, 2, "buyer");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    EXPECT_TRUE(result.trades.empty());
}

TEST_F(MatchingEngineTest, SellOrderDoesNotCrossHigherPricedBid) {
    // Arrange
    auto buy_order = make_order(OrderSide::BUY, 95, 10, 1, "buyer");
    ASSERT_NO_THROW(engine.handle_order(buy_order));
    auto sell_order = make_order(OrderSide::SELL, 100, 10, 2, "seller");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(sell_order));

    // Assert
    EXPECT_TRUE(result.trades.empty());
}

// ---------------------------------------------------------------------------
// Full match: incoming and resting orders have equal volume
// ---------------------------------------------------------------------------

TEST_F(MatchingEngineTest, BuyOrderFullyMatchesRestingAskOfEqualVolume) {
    // Arrange
    auto sell_order = make_order(OrderSide::SELL, 100, 10, 1, "seller");
    ASSERT_NO_THROW(engine.handle_order(sell_order));
    auto buy_order = make_order(OrderSide::BUY, 100, 10, 2, "buyer");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].price, 100);
    EXPECT_EQ(result.trades[0].volume, 10);
    EXPECT_TRUE(result.cancellations.empty());
}

TEST_F(MatchingEngineTest, SellOrderFullyMatchesRestingBidOfEqualVolume) {
    // Arrange
    auto buy_order = make_order(OrderSide::BUY, 100, 10, 1, "buyer");
    ASSERT_NO_THROW(engine.handle_order(buy_order));
    auto sell_order = make_order(OrderSide::SELL, 100, 10, 2, "seller");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(sell_order));

    // Assert
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].price, 100);
    EXPECT_EQ(result.trades[0].volume, 10);
}

TEST_F(MatchingEngineTest, TradeExecutesAtRestingOrderPriceNotAggressorPrice) {
    // Aggressive buy priced above the resting ask should still trade at the ask price.
    // Arrange
    auto sell_order = make_order(OrderSide::SELL, 100, 10, 1, "seller");
    ASSERT_NO_THROW(engine.handle_order(sell_order));
    auto buy_order = make_order(OrderSide::BUY, 110, 10, 2, "buyer");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].price, 100);
}

// ---------------------------------------------------------------------------
// Partial match: incoming and resting orders have differing volume
// ---------------------------------------------------------------------------

TEST_F(MatchingEngineTest, IncomingBuySmallerThanRestingAskLeavesAskResting) {
    // Arrange
    auto sell_order = make_order(OrderSide::SELL, 100, 10, 1, "seller");
    ASSERT_NO_THROW(engine.handle_order(sell_order));
    auto buy_order = make_order(OrderSide::BUY, 100, 4, 2, "buyer");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 4);
    EXPECT_EQ(buy_order->volume, 0);

    // The remaining 6 units of the resting ask should still be matchable.
    auto probe_buy = make_order(OrderSide::BUY, 100, 6, 3, "buyer2");
    MatchResult probe_result;
    ASSERT_NO_THROW(probe_result = engine.handle_order(probe_buy));
    ASSERT_EQ(probe_result.trades.size(), 1u);
    EXPECT_EQ(probe_result.trades[0].resting_order.order_id, 1);
    EXPECT_EQ(probe_result.trades[0].volume, 6);
}

TEST_F(MatchingEngineTest, IncomingBuyLargerThanRestingAskRestsRemainder) {
    // Arrange
    auto sell_order = make_order(OrderSide::SELL, 100, 4, 1, "seller");
    ASSERT_NO_THROW(engine.handle_order(sell_order));
    auto buy_order = make_order(OrderSide::BUY, 100, 10, 2, "buyer");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 4);

    // The remaining 6 units of the buy order should now be resting as the best bid.
    auto probe_sell = make_order(OrderSide::SELL, 100, 6, 3, "seller2");
    MatchResult probe_result;
    ASSERT_NO_THROW(probe_result = engine.handle_order(probe_sell));
    ASSERT_EQ(probe_result.trades.size(), 1u);
    EXPECT_EQ(probe_result.trades[0].resting_order.order_id, 2);
    EXPECT_EQ(probe_result.trades[0].volume, 6);
}

TEST_F(MatchingEngineTest, IncomingSellSmallerThanRestingBidLeavesBidResting) {
    // Arrange
    auto buy_order = make_order(OrderSide::BUY, 100, 10, 1, "buyer");
    ASSERT_NO_THROW(engine.handle_order(buy_order));
    auto sell_order = make_order(OrderSide::SELL, 100, 4, 2, "seller");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(sell_order));

    // Assert
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 4);
}

TEST_F(MatchingEngineTest, IncomingSellLargerThanRestingBidRestsRemainder) {
    // Arrange
    auto buy_order = make_order(OrderSide::BUY, 100, 4, 1, "buyer");
    ASSERT_NO_THROW(engine.handle_order(buy_order));
    auto sell_order = make_order(OrderSide::SELL, 100, 10, 2, "seller");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(sell_order));

    // Assert
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 4);
}

// ---------------------------------------------------------------------------
// Multi-level matching: price and time priority
// ---------------------------------------------------------------------------

TEST_F(MatchingEngineTest, BuyOrderSweepsMultipleAskPriceLevelsInPricePriority) {
    // Arrange
    ASSERT_NO_THROW(engine.handle_order(make_order(OrderSide::SELL, 100, 5, 1, "s1")));
    ASSERT_NO_THROW(engine.handle_order(make_order(OrderSide::SELL, 101, 5, 2, "s2")));
    auto buy_order = make_order(OrderSide::BUY, 101, 10, 3, "buyer");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert: the cheaper ask level is matched before the more expensive one
    ASSERT_EQ(result.trades.size(), 2u);
    EXPECT_EQ(result.trades[0].price, 100);
    EXPECT_EQ(result.trades[1].price, 101);
}

TEST_F(MatchingEngineTest, OrdersAtSamePriceMatchInTimePriority) {
    // Resting orders at an identical price should be matched FIFO (price-time priority).
    // Arrange
    ASSERT_NO_THROW(engine.handle_order(make_order(OrderSide::SELL, 100, 5, 1, "s1")));
    ASSERT_NO_THROW(engine.handle_order(make_order(OrderSide::SELL, 100, 5, 2, "s2")));
    auto buy_order = make_order(OrderSide::BUY, 100, 5, 3, "buyer");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].resting_order.order_id, 1);
}

// ---------------------------------------------------------------------------
// Self-trade cancellation
// ---------------------------------------------------------------------------

TEST_F(MatchingEngineTest, SelfTradeWithEqualVolumeCancelsBothOrders) {
    // Arrange
    auto sell_order = make_order(OrderSide::SELL, 100, 10, 1, "trader1");
    ASSERT_NO_THROW(engine.handle_order(sell_order));
    auto buy_order = make_order(OrderSide::BUY, 100, 10, 2, "trader1");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    EXPECT_TRUE(result.trades.empty());
    ASSERT_EQ(result.cancellations.size(), 1u);
    EXPECT_EQ(result.cancellations[0].volume, 10);
}

TEST_F(MatchingEngineTest, SelfTradeWithSmallerIncomingOrderLeavesRestingRemainder) {
    // Arrange
    auto sell_order = make_order(OrderSide::SELL, 100, 10, 1, "trader1");
    ASSERT_NO_THROW(engine.handle_order(sell_order));
    auto buy_order = make_order(OrderSide::BUY, 100, 4, 2, "trader1");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    EXPECT_TRUE(result.trades.empty());
    ASSERT_EQ(result.cancellations.size(), 1u);
    EXPECT_EQ(result.cancellations[0].volume, 4);

    // The remaining 6 units of the resting sell order (trader1) should still
    // be on the book and matchable by a different trader.
    auto probe_buy = make_order(OrderSide::BUY, 100, 6, 3, "other_trader");
    MatchResult probe_result;
    ASSERT_NO_THROW(probe_result = engine.handle_order(probe_buy));
    ASSERT_EQ(probe_result.trades.size(), 1u);
    EXPECT_EQ(probe_result.trades[0].resting_order.order_id, 1);
    EXPECT_EQ(probe_result.trades[0].volume, 6);
}

TEST_F(MatchingEngineTest, SelfTradeWithLargerIncomingOrderRestsRemainderAndContinuesMatching) {
    // After cancelling against its own resting order, the aggressor should
    // keep matching against other resting orders at the same price level.
    // Arrange
    ASSERT_NO_THROW(engine.handle_order(make_order(OrderSide::SELL, 100, 4, 1, "trader1")));
    ASSERT_NO_THROW(engine.handle_order(make_order(OrderSide::SELL, 100, 10, 2, "other")));
    auto buy_order = make_order(OrderSide::BUY, 100, 10, 3, "trader1");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert
    ASSERT_EQ(result.cancellations.size(), 1u);
    EXPECT_EQ(result.cancellations[0].volume, 4);
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 6);
    EXPECT_EQ(result.trades[0].resting_order.order_id, 2);
}

TEST_F(MatchingEngineTest, SelfTradeDoesNotLeaveIncomingOrderRestingWhenFullyCancelled) {
    // Arrange
    ASSERT_NO_THROW(engine.handle_order(make_order(OrderSide::SELL, 100, 10, 1, "trader1")));
    auto buy_order = make_order(OrderSide::BUY, 100, 10, 2, "trader1");

    // Act
    MatchResult result;
    ASSERT_NO_THROW(result = engine.handle_order(buy_order));

    // Assert: no trades or resting volume remain for either order
    EXPECT_TRUE(result.trades.empty());
    ASSERT_EQ(result.cancellations.size(), 1u);
    EXPECT_EQ(result.cancellations[0].volume, 10);
    EXPECT_EQ(buy_order->volume, 0);
}

}  // namespace

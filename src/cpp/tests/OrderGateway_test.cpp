#include <gtest/gtest.h>

#include <stdexcept>

#include "lob/OrderGateway.h"
#include "lob/Types.h"

namespace {

OrderGateway::OrderRequest make_request(OrderSide side, float price, size_t volume,
                                        const char* trader_id = "TEST") {
    OrderGateway::OrderRequest req{};
    req.side = side;
    req.price = price;
    req.volume = volume;
    req.trader_id = Mpid{trader_id};
    return req;
}

class OrderGatewayTest : public ::testing::Test {
protected:
    OrderGateway gateway;
};

// ---------------------------------------------------------------------------
// Validation: submit_order rejects invalid requests and is_order_valid
// mirrors the same rules
// ---------------------------------------------------------------------------

TEST_F(OrderGatewayTest, SubmitOrderRejectsPriceBelowMinimum) {
    EXPECT_THROW(gateway.submit_order(make_request(OrderSide::BUY, 0.05, 10)), GatewayException);
}

TEST_F(OrderGatewayTest, SubmitOrderRejectsPriceAboveMaximum) {
    EXPECT_THROW(gateway.submit_order(make_request(OrderSide::BUY, 100000.1, 10)), GatewayException);
}

TEST_F(OrderGatewayTest, SubmitOrderRejectsPriceViolatingTickSize) {
    EXPECT_THROW(gateway.submit_order(make_request(OrderSide::BUY, 100.05, 10)), GatewayException);
}

TEST_F(OrderGatewayTest, SubmitOrderRejectsZeroVolume) {
    EXPECT_THROW(gateway.submit_order(make_request(OrderSide::BUY, 100.0, 0)), GatewayException);
}

TEST_F(OrderGatewayTest, SubmitOrderRejectsVolumeAboveMaximum) {
    EXPECT_THROW(gateway.submit_order(make_request(OrderSide::BUY, 100.0, 100001)), GatewayException);
}

TEST_F(OrderGatewayTest, SubmitOrderAcceptsBoundaryPriceAndVolume) {
    EXPECT_NO_THROW(gateway.submit_order(make_request(OrderSide::BUY, 0.1, 1)));
    gateway.reset();
    EXPECT_NO_THROW(gateway.submit_order(make_request(OrderSide::BUY, 100000.0, 100000)));
}

TEST_F(OrderGatewayTest, IsOrderValidMirrorsSubmitOrderRules) {
    EXPECT_FALSE(OrderGateway::is_order_valid(make_request(OrderSide::BUY, 0.05, 10)));
    EXPECT_FALSE(OrderGateway::is_order_valid(make_request(OrderSide::BUY, 100000.1, 10)));
    EXPECT_FALSE(OrderGateway::is_order_valid(make_request(OrderSide::BUY, 100.05, 10)));
    EXPECT_FALSE(OrderGateway::is_order_valid(make_request(OrderSide::BUY, 100.0, 0)));
    EXPECT_FALSE(OrderGateway::is_order_valid(make_request(OrderSide::BUY, 100.0, 100001)));
    EXPECT_TRUE(OrderGateway::is_order_valid(make_request(OrderSide::BUY, 100.0, 10)));
}

TEST_F(OrderGatewayTest, RejectedSubmissionDoesNotConsumeOrderId) {
    EXPECT_THROW(gateway.submit_order(make_request(OrderSide::BUY, 100.0, 0)), GatewayException);

    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 1));
    const Order& order = gateway.get_order(1);
    EXPECT_EQ(order.order_id, 1u);
}

// ---------------------------------------------------------------------------
// Order id assignment
// ---------------------------------------------------------------------------

TEST_F(OrderGatewayTest, OrderIdsIncrementSequentiallyFromOne) {
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 1, "AAAA"));
    gateway.submit_order(make_request(OrderSide::SELL, 105.0, 1, "BBBB"));

    EXPECT_EQ(gateway.get_order(1).order_id, 1u);
    EXPECT_EQ(gateway.get_order(2).order_id, 2u);
}

// ---------------------------------------------------------------------------
// get_order: resting orders are stored internally in ticks
// ---------------------------------------------------------------------------

TEST_F(OrderGatewayTest, RestingOrderPriceIsStoredInTicks) {
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "AAAA"));

    const Order& order = gateway.get_order(1);
    EXPECT_EQ(order.side, OrderSide::BUY);
    EXPECT_EQ(order.price, 1000u);
    EXPECT_EQ(order.volume, 10u);
    EXPECT_EQ(order.trader_id, Mpid{"AAAA"});
}

TEST_F(OrderGatewayTest, GetOrderThrowsForUnknownOrderId) {
    EXPECT_THROW(gateway.get_order(999), std::out_of_range);
}

// ---------------------------------------------------------------------------
// Matching via submit_order: trade prices are real prices (ticks * tick size)
// ---------------------------------------------------------------------------

TEST_F(OrderGatewayTest, BuyOrderRestsOnEmptyBook) {
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10));
    EXPECT_TRUE(result.trades.empty());
    EXPECT_TRUE(result.cancellations.empty());
}

TEST_F(OrderGatewayTest, SellOrderRestsOnEmptyBook) {
    auto result = gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10));
    EXPECT_TRUE(result.trades.empty());
    EXPECT_TRUE(result.cancellations.empty());
}

TEST_F(OrderGatewayTest, BuyOrderDoesNotCrossHigherPricedAsk) {
    gateway.submit_order(make_request(OrderSide::SELL, 105.0, 10, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "BBBB"));

    EXPECT_TRUE(result.trades.empty());
}

TEST_F(OrderGatewayTest, SellOrderDoesNotCrossLowerPricedBid) {
    gateway.submit_order(make_request(OrderSide::BUY, 95.0, 10, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10, "BBBB"));

    EXPECT_TRUE(result.trades.empty());
}

TEST_F(OrderGatewayTest, BuyOrderFullyMatchesRestingAskOfEqualVolume) {
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "BBBB"));

    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_FLOAT_EQ(result.trades[0].price, 100.0f);
    EXPECT_EQ(result.trades[0].volume, 10);
    EXPECT_TRUE(result.cancellations.empty());
}

TEST_F(OrderGatewayTest, SellOrderFullyMatchesRestingBidOfEqualVolume) {
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10, "BBBB"));

    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_FLOAT_EQ(result.trades[0].price, 100.0f);
    EXPECT_EQ(result.trades[0].volume, 10);
}

TEST_F(OrderGatewayTest, TradeExecutesAtRestingOrderPriceNotAggressorPrice) {
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 110.0, 10, "BBBB"));

    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_FLOAT_EQ(result.trades[0].price, 100.0f);
}

TEST_F(OrderGatewayTest, IncomingBuySmallerThanRestingAskLeavesAskResting) {
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 4, "BBBB"));

    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 4);

    const Order& resting = gateway.get_order(1);
    EXPECT_EQ(resting.volume, 6u);

    auto probe = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 6, "CCCC"));
    ASSERT_EQ(probe.trades.size(), 1u);
    EXPECT_EQ(probe.trades[0].resting_order_id, 1u);
    EXPECT_EQ(probe.trades[0].volume, 6);
}

TEST_F(OrderGatewayTest, IncomingBuyLargerThanRestingAskRestsRemainder) {
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 4, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "BBBB"));

    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 4);

    const Order& remainder = gateway.get_order(2);
    EXPECT_EQ(remainder.side, OrderSide::BUY);
    EXPECT_EQ(remainder.volume, 6u);
}

TEST_F(OrderGatewayTest, IncomingSellSmallerThanRestingBidLeavesBidResting) {
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::SELL, 100.0, 4, "BBBB"));

    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 4);

    EXPECT_EQ(gateway.get_order(1).volume, 6u);
}

TEST_F(OrderGatewayTest, IncomingSellLargerThanRestingBidRestsRemainder) {
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 4, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10, "BBBB"));

    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 4);

    const Order& remainder = gateway.get_order(2);
    EXPECT_EQ(remainder.side, OrderSide::SELL);
    EXPECT_EQ(remainder.volume, 6u);
}

TEST_F(OrderGatewayTest, BuyOrderSweepsMultipleAskPriceLevelsInPricePriority) {
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 5, "AAAA"));
    gateway.submit_order(make_request(OrderSide::SELL, 100.1, 5, "BBBB"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.1, 10, "CCCC"));

    ASSERT_EQ(result.trades.size(), 2u);
    EXPECT_FLOAT_EQ(result.trades[0].price, 100.0f);
    EXPECT_FLOAT_EQ(result.trades[1].price, 100.1f);
}

TEST_F(OrderGatewayTest, OrdersAtSamePriceMatchInTimePriority) {
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 5, "AAAA"));
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 5, "BBBB"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 5, "CCCC"));

    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].resting_order_id, 1u);
}

// ---------------------------------------------------------------------------
// Self-trade cancellation
// ---------------------------------------------------------------------------

TEST_F(OrderGatewayTest, SelfTradeWithEqualVolumeCancelsBothOrders) {
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "AAAA"));

    EXPECT_TRUE(result.trades.empty());
    ASSERT_EQ(result.cancellations.size(), 1u);
    EXPECT_EQ(result.cancellations[0].volume, 10);
    EXPECT_FLOAT_EQ(result.cancellations[0].price, 100.0f);
}

TEST_F(OrderGatewayTest, SelfTradeWithSmallerIncomingOrderLeavesRestingRemainder) {
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10, "AAAA"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 4, "AAAA"));

    EXPECT_TRUE(result.trades.empty());
    ASSERT_EQ(result.cancellations.size(), 1u);
    EXPECT_EQ(result.cancellations[0].volume, 4);

    EXPECT_EQ(gateway.get_order(1).volume, 6u);

    auto probe = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 6, "BBBB"));
    ASSERT_EQ(probe.trades.size(), 1u);
    EXPECT_EQ(probe.trades[0].resting_order_id, 1u);
    EXPECT_EQ(probe.trades[0].volume, 6);
}

TEST_F(OrderGatewayTest, SelfTradeWithLargerIncomingOrderContinuesMatchingOthers) {
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 4, "AAAA"));
    gateway.submit_order(make_request(OrderSide::SELL, 100.0, 10, "BBBB"));
    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "AAAA"));

    ASSERT_EQ(result.cancellations.size(), 1u);
    EXPECT_EQ(result.cancellations[0].volume, 4);
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].volume, 6);
    EXPECT_EQ(result.trades[0].resting_order_id, 2u);
    EXPECT_EQ(gateway.get_order(2).volume, 4u);
}

// ---------------------------------------------------------------------------
// Reset
// ---------------------------------------------------------------------------

TEST_F(OrderGatewayTest, ResetClearsBookAndRestartsOrderIds) {
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "AAAA"));
    gateway.submit_order(make_request(OrderSide::SELL, 105.0, 5, "BBBB"));

    gateway.reset();

    EXPECT_THROW(gateway.get_order(1), std::out_of_range);
    EXPECT_THROW(gateway.get_order(2), std::out_of_range);

    auto result = gateway.submit_order(make_request(OrderSide::BUY, 100.0, 3, "CCCC"));
    EXPECT_EQ(gateway.get_order(1).order_id, 1u);
    EXPECT_TRUE(result.trades.empty());

    auto probe = gateway.submit_order(make_request(OrderSide::SELL, 100.0, 3, "DDDD"));
    ASSERT_EQ(probe.trades.size(), 1u);
    EXPECT_EQ(probe.trades[0].volume, 3);
}

// ---------------------------------------------------------------------------
// Snapshot
// ---------------------------------------------------------------------------

TEST_F(OrderGatewayTest, PullSnapshotReturnsEmptyWhileSnapshotsDisabled) {
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "AAAA"));

    BookSnapshot snap = gateway.pull_snapshot();
    EXPECT_TRUE(snap.bids.empty());
    EXPECT_TRUE(snap.asks.empty());
}

}  // namespace
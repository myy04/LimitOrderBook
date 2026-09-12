#include <gtest/gtest.h>

#include <stdexcept>
#include <thread>

#include "lob/OrderGateway.h"
#include "lob/Types.h"

namespace {

OrderRequest make_request(OrderSide side, float price, size_t volume,
                                        const char* trader_id = "TEST") {
    OrderRequest req{};
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
    const OrderRequest& order = gateway.get_order(1);
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
// get_order: orders are returned as OrderRequest with real prices
// (converted back from internal ticks via the OrderRequest(Order) ctor)
// ---------------------------------------------------------------------------

TEST_F(OrderGatewayTest, RestingOrderPriceIsReturnedAsRealPrice) {
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "AAAA"));

    const OrderRequest& order = gateway.get_order(1);
    EXPECT_EQ(order.side, OrderSide::BUY);
    EXPECT_FLOAT_EQ(order.price, 100.0f);
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

    const OrderRequest& resting = gateway.get_order(1);
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

    const OrderRequest& remainder = gateway.get_order(2);
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

    const OrderRequest& remainder = gateway.get_order(2);
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

// Snapshots are refreshed at most once per CONFIG::SNAPSHOT_PERIOD (1s);
// in a fast test only the very first submission pushes one (the engine's
// last_snapshot_time starts at the steady_clock epoch). These tests force a
// second push by sleeping past the period.
TEST_F(OrderGatewayTest, PullSnapshotReturnsBookDepthAfterSnapshotPeriod) {
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "AAAA"));

    BookSnapshot snap = gateway.pull_snapshot();
    ASSERT_FALSE(snap.bids.empty());
    EXPECT_EQ(snap.bids[0].price, 100.0);
    EXPECT_EQ(snap.bids[0].volume, 10);

    // Within the 1s period a second submission must not refresh the snapshot:
    // the SELL order resting at 105.0 is not visible yet.
    gateway.submit_order(make_request(OrderSide::SELL, 105.0, 10, "BBBB"));
    EXPECT_TRUE(gateway.pull_snapshot().asks.empty());

    std::this_thread::sleep_for(CONFIG::SNAPSHOT_PERIOD);
    gateway.submit_order(make_request(OrderSide::BUY, 100.0, 10, "CCCC"));

    snap = gateway.pull_snapshot();
    ASSERT_FALSE(snap.asks.empty());
    EXPECT_EQ(snap.asks[0].price, 105.0);
    EXPECT_EQ(snap.asks[0].volume, 10);
}

TEST_F(OrderGatewayTest, PullSnapshotRespectsDepthLimit) {
    // Ten orders at distinct prices fill the whole depth; an eleventh must
    // not appear in the snapshot (SNAPSHOT_DEPTH = 10). Sleep past the
    // snapshot period first, then submit all orders: the first submission
    // refreshes the snapshot with the (empty) book, so push one more order
    // after a second period to capture the full book.
    std::this_thread::sleep_for(CONFIG::SNAPSHOT_PERIOD);
    for (int i = 0; i < 11; i++) {
        gateway.submit_order(make_request(OrderSide::BUY, 100.0 + i, 10, "AAAA"));
    }

    std::this_thread::sleep_for(CONFIG::SNAPSHOT_PERIOD);
    gateway.submit_order(make_request(OrderSide::SELL, 200.0, 10, "BBBB"));

    BookSnapshot snap = gateway.pull_snapshot();
    EXPECT_EQ(snap.bids.size(), 10);
    ASSERT_FALSE(snap.asks.empty());
    EXPECT_EQ(snap.asks[0].price, 200.0);
}

}  // namespace
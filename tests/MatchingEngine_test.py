import pytest

from LimitOrderBook.MatchingEngine import MatchingEngine
from LimitOrderBook.DataTypes import Order, OrderSide, MatchResult, Trade, SelfTradeCancellation


def make_order(trader_id, side, price, volume, order_id, timestamp=0.0):
    return Order(
        trader_id=trader_id,
        side=side,
        price=price,
        volume=volume,
        order_id=order_id,
        timestamp=timestamp,
    )


@pytest.fixture
def engine():
    return MatchingEngine()


class TestMatchingEngineNoMatch:
    def test_buy_order_rests_on_empty_book(self, engine):
        # Arrange
        buy_order = make_order("trader1", OrderSide.BUY, price=100, volume=10, order_id=1)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert result == MatchResult(trades=[], cancellations=[])
        assert engine.order_book.peek_best_bid() == buy_order
        assert engine.order_book.peek_best_ask() is None

    def test_sell_order_rests_on_empty_book(self, engine):
        # Arrange
        sell_order = make_order("trader1", OrderSide.SELL, price=100, volume=10, order_id=1)

        # Act
        result = engine.handle_order(sell_order)

        # Assert
        assert result == MatchResult(trades=[], cancellations=[])
        assert engine.order_book.peek_best_ask() == sell_order
        assert engine.order_book.peek_best_bid() is None

    def test_buy_order_does_not_cross_lower_priced_ask(self, engine):
        # Arrange
        sell_order = make_order("seller", OrderSide.SELL, price=105, volume=10, order_id=1)
        engine.handle_order(sell_order)
        buy_order = make_order("buyer", OrderSide.BUY, price=100, volume=10, order_id=2)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert result.trades == []
        assert engine.order_book.peek_best_bid() == buy_order
        assert engine.order_book.peek_best_ask() == sell_order

    def test_sell_order_does_not_cross_higher_priced_bid(self, engine):
        # Arrange
        buy_order = make_order("buyer", OrderSide.BUY, price=95, volume=10, order_id=1)
        engine.handle_order(buy_order)
        sell_order = make_order("seller", OrderSide.SELL, price=100, volume=10, order_id=2)

        # Act
        result = engine.handle_order(sell_order)

        # Assert
        assert result.trades == []
        assert engine.order_book.peek_best_ask() == sell_order
        assert engine.order_book.peek_best_bid() == buy_order


class TestMatchingEngineFullMatch:
    def test_buy_order_fully_matches_resting_ask_of_equal_volume(self, engine):
        # Arrange
        sell_order = make_order("seller", OrderSide.SELL, price=100, volume=10, order_id=1)
        engine.handle_order(sell_order)
        buy_order = make_order("buyer", OrderSide.BUY, price=100, volume=10, order_id=2)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert len(result.trades) == 1
        trade = result.trades[0]
        assert trade.price == 100
        assert trade.volume == 10
        assert result.cancellations == []
        assert engine.order_book.peek_best_bid() is None
        assert engine.order_book.peek_best_ask() is None

    def test_sell_order_fully_matches_resting_bid_of_equal_volume(self, engine):
        # Arrange
        buy_order = make_order("buyer", OrderSide.BUY, price=100, volume=10, order_id=1)
        engine.handle_order(buy_order)
        sell_order = make_order("seller", OrderSide.SELL, price=100, volume=10, order_id=2)

        # Act
        result = engine.handle_order(sell_order)

        # Assert
        assert len(result.trades) == 1
        trade = result.trades[0]
        assert trade.price == 100
        assert trade.volume == 10
        assert engine.order_book.peek_best_bid() is None
        assert engine.order_book.peek_best_ask() is None

    def test_trade_executes_at_resting_order_price_not_aggressor_price(self, engine):
        """Aggressive buy priced above the resting ask should still trade at the ask price."""
        # Arrange
        sell_order = make_order("seller", OrderSide.SELL, price=100, volume=10, order_id=1)
        engine.handle_order(sell_order)
        buy_order = make_order("buyer", OrderSide.BUY, price=110, volume=10, order_id=2)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert result.trades[0].price == 100


class TestMatchingEnginePartialMatch:
    def test_incoming_buy_smaller_than_resting_ask_leaves_ask_resting(self, engine):
        # Arrange
        sell_order = make_order("seller", OrderSide.SELL, price=100, volume=10, order_id=1)
        engine.handle_order(sell_order)
        buy_order = make_order("buyer", OrderSide.BUY, price=100, volume=4, order_id=2)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert len(result.trades) == 1
        assert result.trades[0].volume == 4
        assert buy_order.volume == 0
        resting_ask = engine.order_book.peek_best_ask()
        assert resting_ask.order_id == 1
        assert resting_ask.volume == 6
        assert engine.order_book.peek_best_bid() is None

    def test_incoming_buy_larger_than_resting_ask_rests_remainder(self, engine):
        # Arrange
        sell_order = make_order("seller", OrderSide.SELL, price=100, volume=4, order_id=1)
        engine.handle_order(sell_order)
        buy_order = make_order("buyer", OrderSide.BUY, price=100, volume=10, order_id=2)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert len(result.trades) == 1
        assert result.trades[0].volume == 4
        assert engine.order_book.peek_best_ask() is None
        resting_bid = engine.order_book.peek_best_bid()
        assert resting_bid.order_id == 2
        assert resting_bid.volume == 6

    def test_incoming_sell_smaller_than_resting_bid_leaves_bid_resting(self, engine):
        # Arrange
        buy_order = make_order("buyer", OrderSide.BUY, price=100, volume=10, order_id=1)
        engine.handle_order(buy_order)
        sell_order = make_order("seller", OrderSide.SELL, price=100, volume=4, order_id=2)

        # Act
        result = engine.handle_order(sell_order)

        # Assert
        assert len(result.trades) == 1
        assert result.trades[0].volume == 4
        resting_bid = engine.order_book.peek_best_bid()
        assert resting_bid.order_id == 1
        assert resting_bid.volume == 6

    def test_incoming_sell_larger_than_resting_bid_rests_remainder(self, engine):
        # Arrange
        buy_order = make_order("buyer", OrderSide.BUY, price=100, volume=4, order_id=1)
        engine.handle_order(buy_order)
        sell_order = make_order("seller", OrderSide.SELL, price=100, volume=10, order_id=2)

        # Act
        result = engine.handle_order(sell_order)

        # Assert
        assert len(result.trades) == 1
        assert result.trades[0].volume == 4
        assert engine.order_book.peek_best_bid() is None
        resting_ask = engine.order_book.peek_best_ask()
        assert resting_ask.order_id == 2
        assert resting_ask.volume == 6


class TestMatchingEngineMultiLevelMatch:
    def test_buy_order_sweeps_multiple_ask_price_levels_in_price_priority(self, engine):
        # Arrange
        engine.handle_order(make_order("s1", OrderSide.SELL, price=100, volume=5, order_id=1))
        engine.handle_order(make_order("s2", OrderSide.SELL, price=101, volume=5, order_id=2))
        buy_order = make_order("buyer", OrderSide.BUY, price=101, volume=10, order_id=3)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert len(result.trades) == 2
        assert result.trades[0].price == 100
        assert result.trades[1].price == 101
        assert engine.order_book.peek_best_ask() is None
        assert engine.order_book.peek_best_bid() is None

    def test_orders_at_same_price_match_in_time_priority(self, engine):
        """Resting orders at an identical price should be matched FIFO (price-time priority)."""
        # Arrange
        engine.handle_order(make_order("s1", OrderSide.SELL, price=100, volume=5, order_id=1))
        engine.handle_order(make_order("s2", OrderSide.SELL, price=100, volume=5, order_id=2))
        buy_order = make_order("buyer", OrderSide.BUY, price=100, volume=5, order_id=3)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert len(result.trades) == 1
        assert result.trades[0].resting_order.order_id == 1
        remaining_ask = engine.order_book.peek_best_ask()
        assert remaining_ask.order_id == 2


class TestMatchingEngineSelfTrade:
    def test_self_trade_with_equal_volume_cancels_both_orders(self, engine):
        # Arrange
        sell_order = make_order("trader1", OrderSide.SELL, price=100, volume=10, order_id=1)
        engine.handle_order(sell_order)
        buy_order = make_order("trader1", OrderSide.BUY, price=100, volume=10, order_id=2)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert result.trades == []
        assert len(result.cancellations) == 1
        cancellation = result.cancellations[0]
        assert cancellation.volume == 10
        assert engine.order_book.peek_best_ask() is None
        assert engine.order_book.peek_best_bid() is None

    def test_self_trade_with_smaller_incoming_order_leaves_resting_remainder(self, engine):
        # Arrange
        sell_order = make_order("trader1", OrderSide.SELL, price=100, volume=10, order_id=1)
        engine.handle_order(sell_order)
        buy_order = make_order("trader1", OrderSide.BUY, price=100, volume=4, order_id=2)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert result.trades == []
        assert len(result.cancellations) == 1
        assert result.cancellations[0].volume == 4
        resting_ask = engine.order_book.peek_best_ask()
        assert resting_ask.order_id == 1
        assert resting_ask.volume == 6
        assert engine.order_book.peek_best_bid() is None

    def test_self_trade_with_larger_incoming_order_rests_remainder_and_continues_matching(self, engine):
        """After cancelling against its own resting order, the aggressor should keep matching against others."""
        # Arrange
        engine.handle_order(make_order("trader1", OrderSide.SELL, price=100, volume=4, order_id=1))
        engine.handle_order(make_order("other", OrderSide.SELL, price=100, volume=10, order_id=2))
        buy_order = make_order("trader1", OrderSide.BUY, price=100, volume=10, order_id=3)

        # Act
        result = engine.handle_order(buy_order)

        # Assert
        assert len(result.cancellations) == 1
        assert result.cancellations[0].volume == 4
        assert len(result.trades) == 1
        assert result.trades[0].volume == 6
        assert result.trades[0].resting_order.order_id == 2
        remaining_ask = engine.order_book.peek_best_ask()
        assert remaining_ask.order_id == 2
        assert remaining_ask.volume == 4

    def test_self_trade_does_not_leave_incoming_order_resting_when_fully_cancelled(self, engine):
        # Arrange
        engine.handle_order(make_order("trader1", OrderSide.SELL, price=100, volume=10, order_id=1))
        buy_order = make_order("trader1", OrderSide.BUY, price=100, volume=10, order_id=2)

        # Act
        engine.handle_order(buy_order)

        # Assert
        assert engine.order_book.peek_best_bid() is None
        assert engine.order_book.peek_best_ask() is None

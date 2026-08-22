import pytest

from MatchingEngine import MatchingEngine
from DataTypes import Order, OrderSide


def make_order(side, price, volume, order_id, timestamp=0):
    return Order(side=side, price=price, volume=volume, order_id=order_id, timestamp=timestamp)


@pytest.fixture
def engine():
    return MatchingEngine()


class TestNoLiquidity:
    def test_buy_with_empty_book_rests(self, engine):
        order = make_order(OrderSide.BUY, 100, 10, 1)
        trades = engine.handle_order(order)
        assert trades == []
        assert engine.order_book.peek_best_bid().order_id == 1

    def test_sell_with_empty_book_rests(self, engine):
        order = make_order(OrderSide.SELL, 100, 10, 1)
        trades = engine.handle_order(order)
        assert trades == []
        assert engine.order_book.peek_best_ask().order_id == 1


class TestPriceDoesNotCross:
    def test_buy_price_below_best_ask_rests_no_trade(self, engine):
        engine.handle_order(make_order(OrderSide.SELL, 100, 10, 1))
        trades = engine.handle_order(make_order(OrderSide.BUY, 99, 5, 2))
        assert trades == []
        assert engine.order_book.peek_best_ask().order_id == 1
        assert engine.order_book.peek_best_bid().order_id == 2

    def test_sell_price_above_best_bid_rests_no_trade(self, engine):
        engine.handle_order(make_order(OrderSide.BUY, 100, 10, 1))
        trades = engine.handle_order(make_order(OrderSide.SELL, 101, 5, 2))
        assert trades == []
        assert engine.order_book.peek_best_bid().order_id == 1
        assert engine.order_book.peek_best_ask().order_id == 2


class TestExactMatch:
    def test_buy_exact_volume_match_empties_book(self, engine):
        engine.handle_order(make_order(OrderSide.SELL, 100, 10, 1))
        trades = engine.handle_order(make_order(OrderSide.BUY, 100, 10, 2))
        assert len(trades) == 1
        assert trades[0].price == 100
        assert trades[0].volume == 10
        assert engine.order_book.peek_best_ask() is None
        assert engine.order_book.peek_best_bid() is None

    def test_sell_exact_volume_match_empties_book(self, engine):
        engine.handle_order(make_order(OrderSide.BUY, 100, 10, 1))
        trades = engine.handle_order(make_order(OrderSide.SELL, 100, 10, 2))
        assert len(trades) == 1
        assert trades[0].price == 100
        assert trades[0].volume == 10
        assert engine.order_book.peek_best_bid() is None
        assert engine.order_book.peek_best_ask() is None


class TestPartialFills:
    def test_buy_larger_than_resting_ask_rests_remainder(self, engine):
        engine.handle_order(make_order(OrderSide.SELL, 100, 5, 1))
        trades = engine.handle_order(make_order(OrderSide.BUY, 100, 10, 2))
        assert len(trades) == 1
        assert trades[0].volume == 5
        assert engine.order_book.peek_best_ask() is None
        remaining_bid = engine.order_book.peek_best_bid()
        assert remaining_bid.order_id == 2
        assert remaining_bid.volume == 5

    def test_buy_smaller_than_resting_ask_leaves_ask_on_book(self, engine):
        engine.handle_order(make_order(OrderSide.SELL, 100, 10, 1))
        trades = engine.handle_order(make_order(OrderSide.BUY, 100, 4, 2))
        assert len(trades) == 1
        assert trades[0].volume == 4
        assert engine.order_book.peek_best_bid() is None
        remaining_ask = engine.order_book.peek_best_ask()
        assert remaining_ask.order_id == 1
        assert remaining_ask.volume == 6

    def test_sell_larger_than_resting_bid_rests_remainder(self, engine):
        engine.handle_order(make_order(OrderSide.BUY, 100, 5, 1))
        trades = engine.handle_order(make_order(OrderSide.SELL, 100, 10, 2))
        assert len(trades) == 1
        assert trades[0].volume == 5
        assert engine.order_book.peek_best_bid() is None
        remaining_ask = engine.order_book.peek_best_ask()
        assert remaining_ask.order_id == 2
        assert remaining_ask.volume == 5

    def test_sell_smaller_than_resting_bid_leaves_bid_on_book(self, engine):
        engine.handle_order(make_order(OrderSide.BUY, 100, 10, 1))
        trades = engine.handle_order(make_order(OrderSide.SELL, 100, 4, 2))
        assert len(trades) == 1
        assert trades[0].volume == 4
        assert engine.order_book.peek_best_ask() is None
        remaining_bid = engine.order_book.peek_best_bid()
        assert remaining_bid.order_id == 1
        assert remaining_bid.volume == 6


class TestSweepingMultipleLevels:
    def test_buy_sweeps_two_ask_levels_and_rests_remainder(self, engine):
        engine.handle_order(make_order(OrderSide.SELL, 100, 5, 1))
        engine.handle_order(make_order(OrderSide.SELL, 101, 5, 2))
        trades = engine.handle_order(make_order(OrderSide.BUY, 102, 8, 3))

        assert len(trades) == 2
        assert (trades[0].price, trades[0].volume) == (100, 5)
        assert (trades[1].price, trades[1].volume) == (101, 3)

        assert engine.order_book.peek_best_bid() is None
        remaining_ask = engine.order_book.peek_best_ask()
        assert remaining_ask.order_id == 2
        assert remaining_ask.volume == 2

    def test_buy_stops_sweep_at_price_limit(self, engine):
        engine.handle_order(make_order(OrderSide.SELL, 100, 5, 1))
        engine.handle_order(make_order(OrderSide.SELL, 101, 5, 2))
        trades = engine.handle_order(make_order(OrderSide.BUY, 100, 10, 3))

        assert len(trades) == 1
        assert (trades[0].price, trades[0].volume) == (100, 5)

        remaining_bid = engine.order_book.peek_best_bid()
        assert remaining_bid.order_id == 3
        assert remaining_bid.volume == 5

        remaining_ask = engine.order_book.peek_best_ask()
        assert remaining_ask.order_id == 2
        assert remaining_ask.volume == 5

    def test_sell_sweeps_two_bid_levels_and_rests_remainder(self, engine):
        engine.handle_order(make_order(OrderSide.BUY, 101, 5, 1))
        engine.handle_order(make_order(OrderSide.BUY, 100, 5, 2))
        trades = engine.handle_order(make_order(OrderSide.SELL, 99, 8, 3))

        assert len(trades) == 2
        assert (trades[0].price, trades[0].volume) == (101, 5)
        assert (trades[1].price, trades[1].volume) == (100, 3)

        assert engine.order_book.peek_best_ask() is None
        remaining_bid = engine.order_book.peek_best_bid()
        assert remaining_bid.order_id == 2
        assert remaining_bid.volume == 2


class TestFIFOAtSamePriceLevel:
    def test_buy_matches_oldest_ask_first(self, engine):
        engine.handle_order(make_order(OrderSide.SELL, 100, 5, 1, timestamp=1))
        engine.handle_order(make_order(OrderSide.SELL, 100, 5, 2, timestamp=2))

        trades = engine.handle_order(make_order(OrderSide.BUY, 100, 5, 3, timestamp=3))

        assert len(trades) == 1
        assert trades[0].resting_order.order_id == 1
        remaining_ask = engine.order_book.peek_best_ask()
        assert remaining_ask.order_id == 2

    def test_sell_matches_oldest_bid_first(self, engine):
        engine.handle_order(make_order(OrderSide.BUY, 100, 5, 1, timestamp=1))
        engine.handle_order(make_order(OrderSide.BUY, 100, 5, 2, timestamp=2))

        trades = engine.handle_order(make_order(OrderSide.SELL, 100, 5, 3, timestamp=3))

        assert len(trades) == 1
        assert trades[0].resting_order.order_id == 1
        remaining_bid = engine.order_book.peek_best_bid()
        assert remaining_bid.order_id == 2


class TestEdgeCases:
    def test_zero_volume_order_produces_no_trades_and_does_not_rest(self, engine):
        engine.handle_order(make_order(OrderSide.SELL, 100, 5, 1))
        trades = engine.handle_order(make_order(OrderSide.BUY, 100, 0, 2))
        assert trades == []
        # order never rests since the match-loop condition (volume > 0) is false
        assert engine.order_book.peek_best_bid() is None
        assert engine.order_book.peek_best_ask().order_id == 1

    def test_aggressor_and_resting_side_are_tagged_correctly_in_trade(self, engine):
        engine.handle_order(make_order(OrderSide.SELL, 100, 5, 1))
        trades = engine.handle_order(make_order(OrderSide.BUY, 100, 5, 2))
        trade = trades[0]
        assert trade.aggressor_order.order_id == 2
        assert trade.aggressor_order.side == OrderSide.BUY
        assert trade.resting_order.order_id == 1
        assert trade.resting_order.side == OrderSide.SELL

    def test_no_self_cross_when_only_one_side_has_orders(self, engine):
        engine.handle_order(make_order(OrderSide.BUY, 100, 5, 1))
        trades = engine.handle_order(make_order(OrderSide.BUY, 100, 5, 2))
        assert trades == []
        assert len(list(iter([engine.order_book.peek_best_bid()]))) == 1
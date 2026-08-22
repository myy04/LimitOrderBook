import pytest

from LimitOrderBook.OrderGateway import (
    OrderGateway,
    OrderException,
    TickSizeException,
    PriceRangeException,
    VolumeRangeException,
    SideException,
)
from LimitOrderBook.MatchingEngine import MatchingEngine
from LimitOrderBook.DataTypes import OrderSide, MatchResult


@pytest.fixture
def engine():
    return MatchingEngine()


@pytest.fixture
def gateway(engine):
    return OrderGateway(engine)


class TestOrderGatewaySubmitOrderHappyPath:
    def test_submit_buy_order_returns_match_result(self, gateway):
        # Arrange & Act
        result = gateway.submit_order(trader_id="trader1", side="BUY", price=100.50, volume=10)

        # Assert
        assert isinstance(result, MatchResult)
        assert result.trades == []
        assert result.cancellations == []

    def test_submit_sell_order_returns_match_result(self, gateway):
        # Arrange & Act
        result = gateway.submit_order(trader_id="trader1", side="SELL", price=100.50, volume=10)

        # Assert
        assert isinstance(result, MatchResult)
        assert result.trades == []

    def test_submitted_buy_order_rests_on_book_with_expected_fields(self, gateway, engine):
        # Act
        gateway.submit_order(trader_id="trader1", side="BUY", price=100.50, volume=10)

        # Assert
        resting_order = engine.order_book.peek_best_bid()
        assert resting_order.trader_id == "trader1"
        assert resting_order.side == OrderSide.BUY
        assert resting_order.volume == 10
        # price is converted into ticks: 100.50 / 0.01 = 10050
        assert resting_order.price == 10050

    def test_submitted_sell_order_rests_on_book_with_expected_fields(self, gateway, engine):
        # Act
        gateway.submit_order(trader_id="trader1", side="SELL", price=99.99, volume=5)

        # Assert
        resting_order = engine.order_book.peek_best_ask()
        assert resting_order.side == OrderSide.SELL
        assert resting_order.price == 9999
        assert resting_order.volume == 5

    def test_two_crossing_orders_produce_a_trade(self, gateway):
        # Arrange
        gateway.submit_order(trader_id="seller", side="SELL", price=100.00, volume=10)

        # Act
        result = gateway.submit_order(trader_id="buyer", side="BUY", price=100.00, volume=10)

        # Assert
        assert len(result.trades) == 1
        assert result.trades[0].volume == 10


class TestOrderGatewayOrderIdAssignment:
    def test_order_ids_increment_sequentially_starting_at_one(self, gateway):
        """Two resting orders at the same price should be matched FIFO by ascending order id."""
        # Arrange
        gateway.submit_order(trader_id="t1", side="BUY", price=1.00, volume=1)
        gateway.submit_order(trader_id="t2", side="BUY", price=1.00, volume=1)

        # Act
        result = gateway.submit_order(trader_id="t3", side="SELL", price=1.00, volume=1)

        # Assert: the first submitted resting bid (order id 1) is matched first
        assert result.trades[0].resting_order.order_id == 1
        assert result.trades[0].resting_order.trader_id == "t1"

    def test_order_counter_persists_across_sides(self, gateway, engine):
        # Act
        gateway.submit_order(trader_id="t1", side="BUY", price=1.00, volume=1)
        gateway.submit_order(trader_id="t2", side="SELL", price=2.00, volume=1)

        # Assert
        bid = engine.order_book.peek_best_bid()
        ask = engine.order_book.peek_best_ask()
        assert bid.order_id == 1
        assert ask.order_id == 2


class TestOrderGatewaySideValidation:
    @pytest.mark.parametrize("invalid_side", ["buy", "sell", "Buy", "", "HOLD", None, 1])
    def test_invalid_side_raises_side_exception(self, gateway, invalid_side):
        # Act & Assert
        with pytest.raises(SideException):
            gateway.submit_order(trader_id="trader1", side=invalid_side, price=1.00, volume=1)

    @pytest.mark.parametrize("valid_side", ["BUY", "SELL"])
    def test_valid_side_does_not_raise(self, gateway, valid_side):
        # Act & Assert (no exception expected)
        gateway.submit_order(trader_id="trader1", side=valid_side, price=1.00, volume=1)


class TestOrderGatewayTickSizeValidation:
    @pytest.mark.parametrize("price", [1.00, 1.01, 100.50, 0.01, 123.45])
    def test_price_matching_tick_size_does_not_raise(self, gateway, price):
        # Act & Assert (no exception expected)
        gateway.submit_order(trader_id="trader1", side="BUY", price=price, volume=1)

    @pytest.mark.parametrize("price", [1.001, 1.005, 100.999, 0.001])
    def test_price_violating_tick_size_raises_tick_size_exception(self, gateway, price):
        # Act & Assert
        with pytest.raises(TickSizeException):
            gateway.submit_order(trader_id="trader1", side="BUY", price=price, volume=1)

    def test_tick_size_exception_is_an_order_exception(self, gateway):
        """TickSizeException should be catchable as the generic OrderException."""
        # Act & Assert
        with pytest.raises(OrderException):
            gateway.submit_order(trader_id="trader1", side="BUY", price=1.001, volume=1)


class TestOrderGatewayPriceRangeValidation:
    def test_minimum_valid_price_does_not_raise(self, gateway):
        # Act & Assert (0.01 -> 1 tick, the minimum allowed price)
        gateway.submit_order(trader_id="trader1", side="BUY", price=0.01, volume=1)

    def test_price_below_minimum_raises_price_range_exception(self, gateway):
        # Act & Assert
        with pytest.raises(PriceRangeException):
            gateway.submit_order(trader_id="trader1", side="BUY", price=0.00, volume=1)

    def test_maximum_valid_price_does_not_raise(self, gateway):
        # Act & Assert (1e9 ticks * 0.01 tick size == 1e7 price)
        gateway.submit_order(trader_id="trader1", side="BUY", price=1e7, volume=1)

    def test_price_above_maximum_raises_price_range_exception(self, gateway):
        # Act & Assert
        with pytest.raises(PriceRangeException):
            gateway.submit_order(trader_id="trader1", side="BUY", price=1e7 + 0.01, volume=1)

    def test_negative_price_raises_price_range_exception(self, gateway):
        # Act & Assert
        with pytest.raises(PriceRangeException):
            gateway.submit_order(trader_id="trader1", side="BUY", price=-1.00, volume=1)


class TestOrderGatewayVolumeRangeValidation:
    def test_minimum_valid_volume_does_not_raise(self, gateway):
        # Act & Assert
        gateway.submit_order(trader_id="trader1", side="BUY", price=1.00, volume=1)

    def test_volume_below_minimum_raises_volume_range_exception(self, gateway):
        # Act & Assert
        with pytest.raises(VolumeRangeException):
            gateway.submit_order(trader_id="trader1", side="BUY", price=1.00, volume=0)

    def test_maximum_valid_volume_does_not_raise(self, gateway):
        # Act & Assert
        gateway.submit_order(trader_id="trader1", side="BUY", price=1.00, volume=int(1e9))

    def test_volume_above_maximum_raises_volume_range_exception(self, gateway):
        # Act & Assert
        with pytest.raises(VolumeRangeException):
            gateway.submit_order(trader_id="trader1", side="BUY", price=1.00, volume=int(1e9) + 1)

    def test_negative_volume_raises_volume_range_exception(self, gateway):
        # Act & Assert
        with pytest.raises(VolumeRangeException):
            gateway.submit_order(trader_id="trader1", side="BUY", price=1.00, volume=-1)


class TestOrderGatewayValidationOrdering:
    def test_invalid_side_is_checked_before_price_and_volume(self, gateway):
        """Side validation happens first, so an invalid side should raise even if price/volume are also invalid."""
        # Act & Assert
        with pytest.raises(SideException):
            gateway.submit_order(trader_id="trader1", side="INVALID", price=-1.00, volume=-1)

    def test_rejected_order_does_not_increment_order_counter(self, gateway, engine):
        """A failed validation should not consume an order id / reach the matching engine."""
        # Arrange
        with pytest.raises(VolumeRangeException):
            gateway.submit_order(trader_id="trader1", side="BUY", price=1.00, volume=0)

        # Act
        result = gateway.submit_order(trader_id="trader1", side="BUY", price=1.00, volume=1)

        # Assert
        resting_order = engine.order_book.peek_best_bid()
        assert resting_order.order_id == 1
        assert result.trades == []

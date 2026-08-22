import pytest
from decimal import Decimal

from LimitOrderBook.OrderGateway import (
    OrderGateway,
    TickSizeException,
    PriceRangeException,
    VolumeRangeException,
    SideException,
)
from LimitOrderBook.MatchingEngine import MatchingEngine
from LimitOrderBook.DataTypes import OrderSide


@pytest.fixture
def engine():
    return MatchingEngine()


@pytest.fixture
def gateway(engine):
    return OrderGateway(engine=engine)


class TestValidOrders:
    def test_submit_buy_order_returns_empty_trade_list_when_no_match(self, gateway):
        trades = gateway.submit_order("BUY", 100.00, 10)
        assert trades == []

    def test_submit_sell_order_returns_empty_trade_list_when_no_match(self, gateway):
        trades = gateway.submit_order("SELL", 100.00, 10)
        assert trades == []

    def test_order_counter_increments_sequentially(self, gateway):
        gateway.submit_order("BUY", 100.00, 10)
        gateway.submit_order("BUY", 100.00, 10)
        assert gateway._OrderGateway__order_counter == 2

    def test_matching_orders_produce_trade(self, gateway):
        gateway.submit_order("SELL", 100.00, 10)
        trades = gateway.submit_order("BUY", 100.00, 10)
        assert len(trades) == 1
        assert trades[0].volume == 10
        assert trades[0].price == 10000

    def test_price_converted_to_tick_units_in_book(self, gateway):
        gateway.submit_order("BUY", 10.55, 5)
        resting = gateway.engine.order_book.peek_best_bid()
        assert resting.price == 1055


class TestTickSizeValidation:
    def test_price_with_fractional_cents_raises(self, gateway):
        with pytest.raises(TickSizeException):
            gateway.submit_order("BUY", 100.005, 10)

    def test_price_with_many_decimal_places_raises(self, gateway):
        with pytest.raises(TickSizeException):
            gateway.submit_order("BUY", 100.00001, 10)

    def test_valid_two_decimal_place_price_accepted(self, gateway):
        trades = gateway.submit_order("BUY", 100.25, 10)
        assert trades == []


class TestPriceRangeValidation:
    def test_price_at_minimum_boundary_accepted(self, gateway):
        trades = gateway.submit_order("BUY", 0.01, 10)
        assert trades == []

    def test_price_zero_raises(self, gateway):
        with pytest.raises(PriceRangeException):
            gateway.submit_order("BUY", 0.0, 10)

    def test_negative_price_raises(self, gateway):
        with pytest.raises(PriceRangeException):
            gateway.submit_order("BUY", -10.00, 10)

    def test_price_at_maximum_boundary_accepted(self, gateway):
        trades = gateway.submit_order("BUY", 10_000_000.00, 10)
        assert trades == []

    def test_price_above_maximum_raises(self, gateway):
        with pytest.raises(PriceRangeException):
            gateway.submit_order("BUY", 10_000_000.01, 10)


class TestVolumeRangeValidation:
    def test_volume_at_minimum_boundary_accepted(self, gateway):
        trades = gateway.submit_order("BUY", 100.00, 1)
        assert trades == []

    def test_volume_zero_raises(self, gateway):
        with pytest.raises(VolumeRangeException):
            gateway.submit_order("BUY", 100.00, 0)

    def test_negative_volume_raises(self, gateway):
        with pytest.raises(VolumeRangeException):
            gateway.submit_order("BUY", 100.00, -5)

    def test_volume_at_maximum_boundary_accepted(self, gateway):
        trades = gateway.submit_order("BUY", 100.00, 1_000_000_000)
        assert trades == []

    def test_volume_above_maximum_raises(self, gateway):
        with pytest.raises(VolumeRangeException):
            gateway.submit_order("BUY", 100.00, 1_000_000_001)


class TestSideValidation:
    def test_invalid_side_string_raises(self, gateway):
        with pytest.raises(SideException):
            gateway.submit_order("HOLD", 100.00, 10)

    def test_lowercase_side_raises(self, gateway):
        with pytest.raises(SideException):
            gateway.submit_order("buy", 100.00, 10)

    def test_buy_side_maps_to_order_side_enum(self, gateway):
        gateway.submit_order("BUY", 100.00, 10)
        resting = gateway.engine.order_book.peek_best_bid()
        assert resting.side == OrderSide.BUY

    def test_sell_side_maps_to_order_side_enum(self, gateway):
        gateway.submit_order("SELL", 100.00, 10)
        resting = gateway.engine.order_book.peek_best_ask()
        assert resting.side == OrderSide.SELL


class TestValidationFailureState:
    def test_failed_validation_does_not_increment_counter(self, gateway):
        with pytest.raises(SideException):
            gateway.submit_order("INVALID", 100.00, 10)
        with pytest.raises(TickSizeException):
            gateway.submit_order("BUY", 100.001, 10)
        with pytest.raises(PriceRangeException):
            gateway.submit_order("BUY", 0.0, 10)
        with pytest.raises(VolumeRangeException):
            gateway.submit_order("BUY", 100.00, 0)

        assert gateway._OrderGateway__order_counter == 0
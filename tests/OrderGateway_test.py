"""
Test suite for the OrderGateway Python bindings (LimitOrderBook_cpp).

The gateway is the only public entry point into the matching engine:
all scenarios below drive it exclusively through submit_order/get_order/
is_order_valid/reset.
"""
import pytest

import LimitOrderBook as lob
from LimitOrderBook import (
    MatchResult,
    OrderGateway,
    OrderRequest,
    OrderSide,
)


TRADER = lob.Mpid("TEST")
OTHER_TRADER = lob.Mpid("OTHR")


def make_request(side=OrderSide.BUY, price=100.0, volume=10, trader_id=TRADER):
    return OrderRequest(side=side, price=price, volume=volume, trader_id=trader_id)


@pytest.fixture
def gateway():
    return OrderGateway()


class TestOrderGatewayValidation:
    @pytest.mark.parametrize("price", [0.05, -1.0, 100000.1])
    def test_invalid_price_raises_runtime_error(self, gateway, price):
        # Arrange & Act & Assert
        with pytest.raises(RuntimeError):
            gateway.submit_order(make_request(price=price))

    @pytest.mark.parametrize("price", [100.05, 0.15, 100.001])
    def test_price_violating_tick_size_raises_runtime_error(self, gateway, price):
        # Arrange & Act & Assert
        with pytest.raises(RuntimeError):
            gateway.submit_order(make_request(price=price))

    @pytest.mark.parametrize("volume", [0, 100001])
    def test_invalid_volume_raises_runtime_error(self, gateway, volume):
        # Arrange & Act & Assert
        with pytest.raises(RuntimeError):
            gateway.submit_order(make_request(volume=volume))

    def test_boundary_price_and_volume_are_accepted(self, gateway):
        # Arrange & Act & Assert (no exception expected)
        gateway.submit_order(make_request(price=0.1, volume=1))
        gateway.reset()
        gateway.submit_order(make_request(price=100000.0, volume=100000))

    @pytest.mark.parametrize(
        "req, expected",
        [
            (make_request(price=0.05), False),
            (make_request(price=100000.1), False),
            (make_request(price=100.05), False),
            (make_request(volume=0), False),
            (make_request(volume=100001), False),
            (make_request(), True),
        ],
    )
    def test_is_order_valid_mirrors_submit_order_rules(self, gateway, req, expected):
        # Act
        result = OrderGateway.is_order_valid(req)

        # Assert
        assert result is expected

    def test_rejected_submission_does_not_consume_order_id(self, gateway):
        # Arrange
        with pytest.raises(RuntimeError):
            gateway.submit_order(make_request(volume=0))

        # Act
        gateway.submit_order(make_request())

        # Assert
        assert gateway.get_order(1).order_id == 1


class TestOrderGatewayHappyPath:
    def test_submit_buy_order_returns_match_result(self, gateway):
        # Arrange & Act
        result = gateway.submit_order(make_request(side=OrderSide.BUY))

        # Assert
        assert isinstance(result, MatchResult)
        assert result.trades == []
        assert result.cancellations == []

    def test_submit_sell_order_returns_match_result(self, gateway):
        # Arrange & Act
        result = gateway.submit_order(make_request(side=OrderSide.SELL))

        # Assert
        assert isinstance(result, MatchResult)
        assert result.trades == []

    def test_resting_order_price_is_returned_as_real_price(self, gateway):
        # Arrange & Act
        gateway.submit_order(make_request(price=100.0))

        # Assert: internal ticks (100.0 / 0.1 == 1000) are converted back
        # to a real price in the OrderRequest returned to the client
        resting = gateway.get_order(1)
        assert resting.price == pytest.approx(100.0, abs=1e-4)
        assert resting.side == OrderSide.BUY
        assert resting.volume == 10

    def test_order_ids_increment_sequentially(self, gateway):
        # Arrange & Act
        gateway.submit_order(make_request(side=OrderSide.BUY, price=100.0, trader_id=TRADER))
        gateway.submit_order(make_request(side=OrderSide.SELL, price=105.0, trader_id=OTHER_TRADER))

        # Assert
        assert gateway.get_order(1).order_id == 1
        assert gateway.get_order(2).order_id == 2

    def test_get_order_raises_for_unknown_id(self, gateway):
        # Act & Assert: std::out_of_range surfaces as a LookupError via pybind11
        with pytest.raises(LookupError):
            gateway.get_order(999)


class TestOrderGatewayMatching:
    def test_two_crossing_orders_produce_a_trade(self, gateway):
        # Arrange
        gateway.submit_order(make_request(side=OrderSide.SELL, price=100.0, trader_id=OTHER_TRADER))

        # Act
        result = gateway.submit_order(make_request(side=OrderSide.BUY, price=100.0))

        # Assert: prices are real prices (ticks * tick size), so use approx
        assert len(result.trades) == 1
        assert result.trades[0].price == pytest.approx(100.0, abs=1e-4)
        assert result.trades[0].volume == 10
        assert result.trades[0].resting_order_id == 1

    def test_trade_executes_at_resting_order_price(self, gateway):
        # Arrange: aggressive buy priced above the resting ask
        gateway.submit_order(make_request(side=OrderSide.SELL, price=100.0, trader_id=OTHER_TRADER))

        # Act
        result = gateway.submit_order(make_request(side=OrderSide.BUY, price=110.0))

        # Assert
        assert result.trades[0].price == pytest.approx(100.0, abs=1e-4)

    def test_orders_at_same_price_match_in_time_priority(self, gateway):
        # Arrange: two resting asks at the same price from different traders
        gateway.submit_order(make_request(side=OrderSide.SELL, price=100.0, volume=5, trader_id=OTHER_TRADER))
        gateway.submit_order(make_request(side=OrderSide.SELL, price=100.0, volume=5, trader_id=lob.Mpid("THRD")))

        # Act
        result = gateway.submit_order(make_request(side=OrderSide.BUY, price=100.0, volume=5))

        # Assert: first submitted resting ask (order id 1) is matched first
        assert len(result.trades) == 1
        assert result.trades[0].resting_order_id == 1

    def test_buy_order_sweeps_multiple_price_levels(self, gateway):
        # Arrange
        gateway.submit_order(make_request(side=OrderSide.SELL, price=100.0, volume=5, trader_id=OTHER_TRADER))
        gateway.submit_order(make_request(side=OrderSide.SELL, price=100.1, volume=5, trader_id=lob.Mpid("THRD")))

        # Act
        result = gateway.submit_order(make_request(side=OrderSide.BUY, price=100.1, volume=10))

        # Assert: cheaper ask level matched first
        assert len(result.trades) == 2
        assert result.trades[0].price == pytest.approx(100.0, abs=1e-4)
        assert result.trades[1].price == pytest.approx(100.1, abs=1e-4)

    def test_partial_fill_leaves_resting_order_remainder(self, gateway):
        # Arrange
        gateway.submit_order(make_request(side=OrderSide.SELL, price=100.0, volume=10, trader_id=OTHER_TRADER))

        # Act
        result = gateway.submit_order(make_request(side=OrderSide.BUY, price=100.0, volume=4))

        # Assert
        assert result.trades[0].volume == 4
        assert gateway.get_order(1).volume == 6


class TestOrderGatewaySelfTrade:
    def test_self_trade_cancels_instead_of_trading(self, gateway):
        # Arrange: same trader on both sides
        gateway.submit_order(make_request(side=OrderSide.SELL, price=100.0))

        # Act
        result = gateway.submit_order(make_request(side=OrderSide.BUY, price=100.0))

        # Assert
        assert result.trades == []
        assert len(result.cancellations) == 1
        assert result.cancellations[0].volume == 10
        assert result.cancellations[0].price == pytest.approx(100.0, abs=1e-4)


class TestOrderGatewayReset:
    def test_reset_clears_book_and_restarts_order_ids(self, gateway):
        # Arrange
        gateway.submit_order(make_request(side=OrderSide.BUY, price=100.0))
        gateway.submit_order(make_request(side=OrderSide.SELL, price=105.0, trader_id=OTHER_TRADER))

        # Act
        gateway.reset()

        # Assert: old orders are gone (std::out_of_range surfaces as a LookupError)
        with pytest.raises(LookupError):
            gateway.get_order(1)
        with pytest.raises(LookupError):
            gateway.get_order(2)

        # Assert: fresh submission starts again from order id 1
        gateway.submit_order(make_request(side=OrderSide.BUY, price=100.0))
        assert gateway.get_order(1).order_id == 1
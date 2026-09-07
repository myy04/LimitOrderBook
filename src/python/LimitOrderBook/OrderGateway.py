from LimitOrderBook import PRICE_TICK_SIZE
import time 

from decimal import Decimal

from .MatchingEngine import MatchingEngine
from .DataTypes import *

from . import LimitOrderBook_cpp as cpp 

class OrderException(Exception):
    def __init__(self):
        pass

class TickSizeException(OrderException):
    def __init__(self):
        pass

class PriceRangeException(OrderException):
    def __init__(self):
        pass

class VolumeRangeException(OrderException):
    def __init__(self):
        pass

class SideException(OrderException):
    def __init__(self):
        pass


@dataclass
class OrderRequest:
    side: OrderSide
    price: float
    volume: int
    trader_id: str

class OrderGateway:
    def __init__(self, engine: MatchingEngine = None):
        self.__order_counter = 0
        self.engine: MatchingEngine = engine if engine is not None else MatchingEngine()
        
        self.__TICK_SIZE: Decimal = Decimal(str(PRICE_TICK_SIZE))

        self.__MIN_PRICE: int = 1
        self.__MAX_PRICE: int = int(1e9)
        self.__MIN_VOLUME: int = 1
        self.__MAX_VOLUME: int = int(1e9)

    def submit_order_request(self, order_request: OrderRequest) -> MatchResult:
        order = self.__construct_order(trader_id=order_request.trader_id, side=order_request.side, price=order_request.price, volume=order_request.volume) 
        return self.engine.handle_order(order)

    def submit_order(self, trader_id, side, price, volume) -> MatchResult:
        order = self.__construct_order(trader_id=trader_id, side=side, price=price, volume=volume) 
        return self.engine.handle_order(order)

    def __construct_order(self, trader_id: str, side: str, price: float, volume: int) -> Order: 
        if side != "BUY" and side != "SELL": raise SideException()
        side = OrderSide.BUY if side == "BUY" else OrderSide.SELL

        price = Decimal(str(price))
        if price % self.__TICK_SIZE != 0: raise TickSizeException()
        price = int(price / self.__TICK_SIZE)
        if price < self.__MIN_PRICE or price > self.__MAX_PRICE: raise PriceRangeException()

        volume = int(volume)
        if volume < self.__MIN_VOLUME or volume > self.__MAX_VOLUME: raise VolumeRangeException()

        order_id = self.__order_counter + 1
        self.__order_counter += 1

        if isinstance(self.engine, MatchingEngine): order = Order(trader_id = trader_id, side = side, price = price, volume = volume, order_id = order_id, timestamp = time.time())
        elif isinstance(self.engine, cpp.MatchingEngine): 
            order = cpp.Order(
                trader_id = trader_id, 
                side = cpp.OrderSide.BUY if side == OrderSide.BUY else cpp.OrderSide.SELL,
                price = price,
                volume = volume,
                order_id = order_id,
                timestamp = float(time.time())
            )
        else:
            raise Exception("Engine is of wrong type")

        return order

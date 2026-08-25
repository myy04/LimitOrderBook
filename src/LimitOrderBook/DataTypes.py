from __future__ import annotations
from dataclasses import dataclass 
from enum import Enum

class OrderSide(Enum):
    SELL = 0
    BUY = 1

@dataclass
class Order:
    side: OrderSide
    price: int
    volume: int
    order_id: int
    timestamp: float
    trader_id: str

@dataclass
class Trade:
    aggressor_order: Order
    resting_order: Order
    price: int
    volume: int

@dataclass
class SelfTradeCancellation:
    aggressor_order: Order
    resting_order: Order
    price: int
    volume: int

@dataclass
class MatchResult:
    trades: list[Trade]
    cancellations: list[SelfTradeCancellation]

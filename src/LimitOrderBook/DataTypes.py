from __future__ import annotations
from dataclasses import dataclass 
from enum import Enum

class OrderSide(Enum):
    SELL = 1
    BUY = 0

@dataclass
class Order:
    side: OrderSide
    price: int
    volume: int
    order_id: int
    timestamp: int


@dataclass
class Trade:
    aggressor_order: Order
    resting_order: Order
    price: int
    volume: int

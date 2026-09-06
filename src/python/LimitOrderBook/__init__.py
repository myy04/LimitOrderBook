import decimal
PRICE_TICK_SIZE = decimal.Decimal("0.01")
SNAPSHOT_PERIOD = 1
SNAPSHOT_DEPTH = 10

from LimitOrderBook.DataTypes import *
from LimitOrderBook.MatchingEngine import *
from LimitOrderBook.OrderGateway import * 

from . import LimitOrderBook_cpp as cpp
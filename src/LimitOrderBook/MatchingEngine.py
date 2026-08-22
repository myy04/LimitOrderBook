from __future__ import annotations

from copy import copy

from .OrderBook import OrderBook
from .DataTypes import *


class MatchingEngine:
    def __init__(self):
        self.order_book = OrderBook()
    
    def handle_order(self, order: Order) -> MatchResult:
        if order.side == OrderSide.BUY: return self.__handle_buy(order)
        else: return self.__handle_sell(order)

    def __handle_buy(self, buy_order: Order) -> MatchResult:
        trades = []
        cancellations = []

        while buy_order.volume > 0:
            best_ask = self.order_book.peek_best_ask()
            if best_ask is None or best_ask.price > buy_order.price: break

            if best_ask.trader_id == buy_order.trader_id:
                cancellations += [self.__handle_self_trade(aggressor_order=buy_order, resting_order = best_ask)]
                continue

            if best_ask.volume <= buy_order.volume:
                trade = Trade(aggressor_order=copy(buy_order), resting_order=copy(best_ask), price=best_ask.price, volume=best_ask.volume)
                buy_order.volume -= best_ask.volume
                best_ask.volume = 0
                self.order_book.remove_order(best_ask)
                trades += [trade]
            else:
                trade = Trade(aggressor_order=copy(buy_order), resting_order=copy(best_ask), price=best_ask.price, volume=buy_order.volume) 
                best_ask.volume -= buy_order.volume
                buy_order.volume = 0
                trades += [trade]

        if buy_order.volume > 0:
            self.order_book.insert_order(buy_order)
        return MatchResult(trades=trades, cancellations=cancellations)

    def __handle_sell(self, sell_order: Order) -> MatchResult:
        trades = []
        cancellations = []

        while sell_order.volume > 0:
            best_bid = self.order_book.peek_best_bid()
            if best_bid is None or best_bid.price < sell_order.price: break

            if best_bid.trader_id == sell_order.trader_id:
                cancellations += [self.__handle_self_trade(aggressor_order=sell_order, resting_order=best_bid)]
                continue

            if best_bid.volume <= sell_order.volume:
                trade = Trade(aggressor_order=copy(sell_order), resting_order=copy(best_bid), price=best_bid.price, volume=best_bid.volume)
                sell_order.volume -= best_bid.volume
                best_bid.volume = 0
                self.order_book.remove_order(best_bid)
                trades += [trade]
            else:
                trade = Trade(aggressor_order=copy(sell_order), resting_order=copy(best_bid), price=best_bid.price, volume=sell_order.volume)
                best_bid.volume -= sell_order.volume
                sell_order.volume = 0
                trades += [trade]

        if sell_order.volume > 0:
            self.order_book.insert_order(sell_order)
        return MatchResult(trades=trades, cancellations=cancellations)


    def __handle_self_trade(self, aggressor_order: Order, resting_order: Order) -> SelfTradeCancellation:
        overlap = min(aggressor_order.volume, resting_order.volume) 
        cancel = SelfTradeCancellation(aggressor_order=copy(aggressor_order), resting_order=copy(resting_order), price=resting_order.price, volume=overlap)
        resting_order.volume -= overlap
        aggressor_order.volume -= overlap
        
        if resting_order.volume == 0:
            self.order_book.remove_order(resting_order)

        return cancel

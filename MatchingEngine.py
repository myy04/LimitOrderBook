from copy import copy

from OrderBook import OrderBook
from DataTypes import Order, Trade, OrderSide


class MatchingEngine:
    def __init__(self):
        self.order_book = OrderBook()
    
    def handle_order(self, order: Order):
        if order.side == OrderSide.BUY: return self.__handle_buy(order)
        else: return self.__handle_sell(order)

    def __handle_buy(self, buy_order):  
        trades = []
        while buy_order.volume > 0:
            best_ask = self.order_book.peek_best_ask()
            if best_ask is None or best_ask.price > buy_order.price: break
            
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
        return trades

    def __handle_sell(self, sell_order):
        trades = []
        while sell_order.volume > 0:
            best_bid = self.order_book.peek_best_bid()
            if best_bid is None or best_bid.price < sell_order.price: break

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
        return trades
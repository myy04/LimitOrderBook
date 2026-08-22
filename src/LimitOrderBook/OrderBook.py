from __future__ import annotations

from sortedcontainers import SortedDict

from .DataTypes import Order, Trade, OrderSide
from .DoublyLinkedList import Node, DoublyLinkedList

class OrderBook:
    def __init__(self):
        self.__bids = SortedDict() 
        self.__asks = SortedDict()
        self.__nodes = {}

    def insert_order(self, order: Order) -> None:
        if order.side == OrderSide.BUY:
            try: dll = self.__bids[order.price]
            except: 
                dll = DoublyLinkedList() 
                self.__bids.update({order.price : dll})
        else:
            try: dll = self.__asks[order.price]
            except:
                dll = DoublyLinkedList()
                self.__asks.update({order.price : dll})

        node = Node(data=order)
        dll.append(node)
        self.__nodes.update({order.order_id : node})

    def remove_order(self, order: Order) -> None:
        try:
            node = self.__nodes.pop(order.order_id)
            if order.side == OrderSide.BUY:
                dll = self.__bids[order.price]
            else:
                dll = self.__asks[order.price]
        except:
            return

        dll.remove(node)
        if len(dll) == 0:
            if order.side == OrderSide.BUY: self.__bids.pop(order.price)
            else: self.__asks.pop(order.price)

    def peek_best_bid(self) -> Any:
        try:
            dll = self.__bids.peekitem(index = -1)[1]
            front_node = dll.front()
            return front_node.data
        except:
            return None

    def peek_best_ask(self) -> Any: 
        try:
            dll = self.__asks.peekitem(index = 0)[1]
            front_node = dll.front()
            return front_node.data
        except:
            return None


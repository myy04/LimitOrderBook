from DataTypes import Order, Trade, OrderSide
from sortedcontainers import SortedDict
from DoublyLinkedList import Node, DoublyLinkedList

class OrderBook:
    def __init__(self):
        self.__bids = SortedDict()
        self.__asks = SortedDict()
        self.__nodes = {}

    def insert_order(self, order: Order):
        if order.side == OrderSide.BUY:
            dll = self.__bids.get(order.price, default = DoublyLinkedList())
        else:
            dll = self.__asks.get(order.price, default = DoublyLinkedList())

        node = Node(data=Order)
        dll.append(node)
        self.__nodes.update({order.order_id : node})

    def remove_order(self, order: Order): 
        node = self.__nodes.pop(order.order_id)
        if order.side == OrderSide.BUY:
            dll = self.__bids.get(order.price)
        else:
            dll = self.__asks.get(order.price)
        
        dll.remove(node)
        if len(dll) == 0:
            if order.side == OrderSide.BUY: self.__bids.pop(dll)
            else: self.__asks.pop(dll)


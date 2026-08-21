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
            try: dll = self.__bids.get(order.price)
            except: 
                dll = DoublyLinkedList() 
                self.__bids.update({order.price : dll})
        else:
            try: dll = self.__asks.get(order.price)
            except:
                dll = DoublyLinkedList()
                self.__asks.update({order.price : dll})

        node = Node(data=order)
        dll.append(node)
        self.__nodes.update({order.order_id : node})

    def remove_order(self, order: Order): 
        try: node = self.__nodes.pop(order.order_id)
        except: return

        if order.side == OrderSide.BUY:
            dll = self.__bids.get(order.price)
        else:
            dll = self.__asks.get(order.price)
        
        dll.remove(node)
        if len(dll) == 0:
            if order.side == OrderSide.BUY: self.__bids.pop(dll)
            else: self.__asks.pop(dll)

    def peek_best_bid(self):
        dll = self.__bids.peekitem(index = -1) 
        front_node = dll.front()
        return front_node.data

    def peek_best_ask(self): 
        dll = self.__asks.peekitem(index = 0)
        front_node = dll.front()
        return front_node.data


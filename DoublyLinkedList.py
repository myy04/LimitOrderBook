from dataclasses import dataclass

@dataclass
class Node:
    data: Any = None
    _left: Node = None
    _right: Node = None

class DoublyLinkedList:
    def __init__(self):
        self.__root = Node()
        self.__tail = Node()    
        self.__root._right = self.__tail
        self.__tail._left = self.__root
        self.__size = 0

    def append(self, node: Node):
        self.__tail._right = node
        self.__tail = self.__tail._right
        self.__size += 1

    def append_after(self, old_node: Node, new_node: Node):
        new_node._right = old_node._right
        old_node._right = new_node
        new_node._left = old_node    
        self.__size += 1

    def remove(self, node):
        node._left._right = node._right
        node._right._left = node._left
        self.__size -= 1
    
    def front(self): 
        return self.__root._right

    def __len__(self):
        return self.__size

from dataclasses import dataclass
from typing import Any

@dataclass(eq=False)
class Node:
    data: Any = None
    _left: Node = None
    _right: Node = None

    def __repr__(self):
        return str(self.data)

class DoublyLinkedList:
    def __init__(self):
        self.__root = Node()
        self.__tail = Node()    
        self.__root._right = self.__tail
        self.__tail._left = self.__root
        self.__size = 0

    def append(self, node: Node):
        self.append_after(self.__tail._left, node) 

    def append_after(self, old_node: Node, new_node: Node):
        new_node._right = old_node._right
        old_node._right._left = new_node

        old_node._right = new_node
        new_node._left = old_node    
        self.__size += 1

    def remove(self, node):
        node._left._right = node._right
        node._right._left = node._left
        self.__size -= 1
    
    def front(self): 
        if self.__size == 0: return None
        return self.__root._right

    def __len__(self):
        return self.__size

    def __iter__(self):
        class iterable:
            def __init__(self, first, last): 
                self.current_iter = first
                self.last_iter = last
            def __next__(self):
                if self.current_iter == self.last_iter or self.current_iter._right == self.last_iter:
                    raise StopIteration
                else:
                    self.current_iter = self.current_iter._right
                    return self.current_iter

        return iterable(self.__root, self.__tail)

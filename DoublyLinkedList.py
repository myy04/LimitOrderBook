from dataclasses import dataclass

@dataclass
class Node:
    data: None
    __left: Node = None
    __right: Node = None

class DoublyLinkedList:
    def __init__(self):
        self.__root = Node()
        self.__tail = Node()    

        self.__root.right = self.__tail
        self.__tail.left = self.__root

        self.size = 0
    
    def append(self, node: Node):
        self.__tail.right = Node
        self.__tail = self.__tail.right
        self.size += 1

    def append_after(self, old_node: Node, new_node: Node):
        new_node.right = old_node.right
        old_node.right = new_node
        new_node.left = old_node    
        self.size += 1

    def remove(self, node):
        node.left.right = node.right
        node.right.left = node.left
        self.size -= 1
    
    def __len__(self):
        return self.size

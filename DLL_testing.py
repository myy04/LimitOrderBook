import pytest
from DoublyLinkedList import DoublyLinkedList, Node


class TestNode:
    def test_default_construction(self):
        node = Node()
        assert node.data is None
        assert node._left is None
        assert node._right is None

    def test_repr(self):
        node = Node(42)
        assert repr(node) == "42"


class TestDoublyLinkedList:
    def test_empty_list_len(self):
        dll = DoublyLinkedList()
        assert len(dll) == 0

    def test_empty_list_iteration(self):
        dll = DoublyLinkedList()
        assert list(dll) == []

    def test_append_increases_length(self):
        dll = DoublyLinkedList()
        for i in range(5):
            assert len(dll) == i
            dll.append(Node(i))
            assert len(dll) == i + 1

    def test_append_order_via_iteration(self):
        dll = DoublyLinkedList()
        values = [1, 2, 3]
        for v in values:
            dll.append(Node(v))
        assert [n.data for n in dll] == values

    def test_front_returns_first_node(self):
        dll = DoublyLinkedList()
        dll.append(Node("a"))
        dll.append(Node("b"))
        assert dll.front().data == "a"

    def test_append_after(self):
        dll = DoublyLinkedList()
        first = Node(1)
        dll.append(first)
        dll.append(Node(3))
        dll.append_after(first, Node(2))
        assert [n.data for n in dll] == [1, 2, 3]

    def test_remove_middle_node(self):
        dll = DoublyLinkedList()
        nodes = [Node(i) for i in range(3)]
        for n in nodes:
            dll.append(n)
        dll.remove(nodes[1])
        assert len(dll) == 2
        assert [n.data for n in dll] == [0, 2]

    def test_remove_updates_front(self):
        dll = DoublyLinkedList()
        first = Node(1)
        dll.append(first)
        dll.append(Node(2))
        dll.remove(first)
        assert dll.front().data == 2

    def test_remove_all_nodes(self):
        dll = DoublyLinkedList()
        nodes = [Node(i) for i in range(3)]
        for n in nodes:
            dll.append(n)
        for n in nodes:
            dll.remove(n)
        assert len(dll) == 0
        assert list(dll) == []
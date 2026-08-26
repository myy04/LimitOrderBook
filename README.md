# Limit Order Book

This is my own implementation of Limit Order Book and Maching Engine in Python and C++. 

# Pipeline

OrderGateway provides the public API for submitting orders via OrderGateway.submit_orders() or OrderGateway.submit_order_request().
The gateway checks if the order request is valid (price range, volume range, divisibility by price_tick), constructs a new valid older, and sends it to the matching engine. 
The matching engine matches the aggressor order and the resting orders from the order book and returns the list of completed trades.

MatchingEngine and OrderBook have implementations in Python and C++. 
The C++ implementations can be used in Python code via pybind11.

Here is the performance difference (Python Engine vs CPP Engine):

yerdaulet@Yerdaulets-MacBook-Pro: ~/Development/LimitOrderBook main ⚡
$ python benchmark.py                                                                                       
Orders: 100
Python: 0.0006763339042663574
CPP: 0.00019420799799263477
----------------------------------------------------------------------------------------------------
Orders: 10000
Python: 0.03685137489810586
CPP: 0.015233041951432824
----------------------------------------------------------------------------------------------------
Orders: 1000000
Python: 3.839766375022009
CPP: 1.5110946670174599
----------------------------------------------------------------------------------------------------
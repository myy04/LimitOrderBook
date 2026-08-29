# Limit Order Book

This is my own implementation of Limit Order Book and Maching Engine in Python and C++. 

![Screenshot](./images/cli_screenshot.png)

### Pipeline

-- OrderGateway provides the public API for submitting orders via OrderGateway.submit_orders() or OrderGateway.submit_order_request().

-- The gateway checks if the order request is valid (price range, volume range, divisibility by price_tick), constructs a new valid older, and sends it to the matching engine. 

-- The matching engine matches the aggressor order and the resting orders from the order book and returns the list of completed trades. If the agressor order is not completely filled, it is put in the order book. 

-- MatchingEngine and OrderBook have implementations in Python and C++. The  C++ implementations can be used in Python code via pybind11.

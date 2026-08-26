import LimitOrderBook as lob
from OrderGenerator import OrderGenerator 

import time

def benchmark_python(num_orders: int): 
    engine = lob.MatchingEngine()
    order_gateway = lob.OrderGateway(engine=engine)
    order_generator = OrderGenerator()
    t0 = time.perf_counter()
    for order in order_generator:
        order_gateway.submit_order_request(order)
        num_orders -= 1
        if num_orders == 0: break

    t1 = time.perf_counter() - t0
    return t1

def benchmark_cpp(num_orders: int):
    engine = lob.cpp.MatchingEngine()
    order_gateway = lob.OrderGateway(engine=engine)
    order_generator = OrderGenerator()
    t0 = time.perf_counter()
    for order in order_generator:
        order_gateway.submit_order_request(order)
        num_orders -= 1
        if num_orders == 0: break
    t1 = time.perf_counter() - t0
    return t1


if __name__ == "__main__":

    for num_orders in [int(1e2), int(1e4), int(1e6)]:
        

        print("Orders:", num_orders)
        print("Python:", benchmark_python(num_orders))
        print("CPP:", benchmark_cpp(num_orders))

        print("-" * 100)

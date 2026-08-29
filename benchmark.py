import LimitOrderBook as lob
from OrderGenerator import OrderGenerator 

import time

def benchmark_python(num_orders: int): 
    engine = lob.MatchingEngine()
    order_gateway = lob.OrderGateway(engine=engine)
    order_generator = OrderGenerator()
    t0 = time.perf_counter()
    for order in order_generator:
        try:
            order_gateway.submit_order_request(order)
            num_orders -= 1
            if num_orders == 0: break
        except lob.OrderException:
            continue
        except:
            break
    t1 = time.perf_counter() - t0
    return t1

def benchmark_cpp(num_orders: int):
    engine = lob.cpp.MatchingEngine()
    order_gateway = lob.OrderGateway(engine=engine)
    order_generator = OrderGenerator()
    t0 = time.perf_counter()
    for order in order_generator:
        try:
            order_gateway.submit_order_request(order)
            num_orders -= 1
            if num_orders == 0: break
        except lob.OrderException:
            continue
        except:
            break
    t1 = time.perf_counter() - t0
    return t1


if __name__ == "__main__":

    for num_orders in [int(1e2), int(1e4), int(1e5)]:
        

        print("Orders:", num_orders)
        cpp_result = benchmark_cpp(num_orders)
        print("CPP:", cpp_result)
        python_result = benchmark_python(num_orders)
        print("Python:", python_result)
        print("Python/CPP ratio:", python_result / cpp_result)


        print("-" * 100)

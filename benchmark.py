import LimitOrderBook as lob

import random, time

from decimal import Decimal


class Dataset:
    def __init__(self, length : int, seed : int=47):
        self.random = random.Random(seed)
        self.__dataset: list[lob.OrderRequest] = [self.__generate_order_request() for i in range(length)]

    def __iter__(self):
        return iter(self.__dataset)

    def __generate_order_request(self) -> lob.OrderRequest:
        POSSIBLE_TRADER_IDS: list[str] = [
            "USB",
            "JOHN",
            "GOLDMAN_STANLEY",
            "MORGAN_SACHS",
            "BJ_PENN",
            "FALL",
            "WRIGLEYS"
        ]

        PRICE_DIST_MEAN = 100
        PRICE_DIST_SD = 10

        VOLUME_DIST_MEAN = 10000
        VOLUME_DIST_SD = 1000

        order_request = lob.OrderRequest(
            side = "BUY" if self.random.randint(0, 1) else "SELL",
            price = Decimal(str(int(self.random.gauss(PRICE_DIST_MEAN, PRICE_DIST_SD)))) * Decimal(lob.PRICE_TICK_SIZE),
            volume = int(self.random.gauss(VOLUME_DIST_MEAN, VOLUME_DIST_SD)),
            trader_id = POSSIBLE_TRADER_IDS[self.random.randint(0, len(POSSIBLE_TRADER_IDS) - 1)]
        )

        return order_request


def benchmark_python(dataset): 
    engine = lob.MatchingEngine()
    order_gateway = lob.OrderGateway(engine=engine)
    t0 = time.perf_counter()
    for order_request in dataset:
        order_gateway.submit_order_request(order_request)
    t1 = time.perf_counter() - t0
    return t1

def benchmark_cpp(dataset):
    engine = lob.cpp.MatchingEngine()
    order_gateway = lob.OrderGateway(engine=engine)
    t0 = time.perf_counter()
    for order_request in dataset:
        order_gateway.submit_order_request(order_request)
    t1 = time.perf_counter() - t0
    return t1



if __name__ == "__main__":

    for dataset_length in [int(1e2), int(1e4), int(1e6)]:
        dataset = Dataset(dataset_length)

        print("Orders:", dataset_length)
        print("Python:", benchmark_python(dataset))
        print("CPP:", benchmark_cpp(dataset))

        print("-" * 100)

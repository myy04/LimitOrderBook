from LimitOrderBook import MatchingEngine
import LimitOrderBook as lob
import random

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
            price = int(self.random.gauss(PRICE_DIST_MEAN, PRICE_DIST_SD)) * lob.PRICE_TICK_SIZE,
            volume = int(self.random.gauss(VOLUME_DIST_MEAN, VOLUME_DIST_SD)),
            trader_id = POSSIBLE_TRADER_IDS[self.random.randint(0, len(POSSIBLE_TRADER_IDS) - 1)]
        )

        return order_request


if __name__ == "__main__":
    dataset = Dataset(10)
    order_gateway = lob.OrderGateway()
    trades: list[lob.Trade] = []
    cancellations: list[lob.SelfTradeCancellation] = []

    for order_request in dataset:
        print("Order:", order_request)
        try:
            match: lob.MatchResult = order_gateway.submit_order_request(order_request)
            trades.extend(match.trades)
            cancellations.extend(match.cancellations)
        except Exception as e:
            print(type(e), e)

    print("Trades:", trades)

    print("Cancellations:", cancellations)
            
    
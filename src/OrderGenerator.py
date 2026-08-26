import LimitOrderBook as lob


import random
from decimal import Decimal


class OrderGenerator:
    def __init__(self, seed : int=47):
        self.random = random.Random(seed)   

    def __iter__(self) -> lob.OrderRequest:
        POSSIBLE_TRADER_IDS: list[str] = [
            "USB",
            "JOHN",
            "GOLDMAN_STANLEY",
            "MORGAN_SACHS",
            "BJ_PENN",
            "FALL",
            "WRIGLEYS"
        ]

        PRICE_DIST_MEAN = 100000
        PRICE_DIST_SD = 100

        VOLUME_DIST_MEAN = 10000
        VOLUME_DIST_SD = 1000

        while True:
            PRICE_DIST_MEAN = int(self.random.gauss(PRICE_DIST_MEAN, PRICE_DIST_SD))
            PRICE_DIST_SD = 100

            order_request = lob.OrderRequest(
                side = "BUY" if self.random.randint(0, 1) else "SELL",
                price = Decimal(str(int(self.random.gauss(PRICE_DIST_MEAN, PRICE_DIST_SD)))) * Decimal(lob.PRICE_TICK_SIZE),
                volume = int(self.random.gauss(VOLUME_DIST_MEAN, VOLUME_DIST_SD)),
                trader_id = POSSIBLE_TRADER_IDS[self.random.randint(0, len(POSSIBLE_TRADER_IDS) - 1)]
            )

            yield order_request
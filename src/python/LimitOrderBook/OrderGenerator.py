import random
import struct

import LimitOrderBook as lob


def _f32(x: float) -> float:
    """Round a Python float to the nearest C++ float (32-bit) value."""
    return struct.unpack("f", struct.pack("f", x))[0]


# Mirrors CONFIG as the engine sees it: 32-bit floats. Prices must be
# constructed in 32-bit arithmetic (ticks * TICK) or the gateway's
# tick-size validation can reject them for being one ULP off.
TICK = lob.PRICE_TICK_SIZE
MIN_TICKS = round(lob.MIN_PRICE / TICK)
MAX_TICKS = round(lob.MAX_PRICE / TICK)


class OrderGenerator:
    def __init__(self, seed: int = 47):
        self.random = random.Random(seed)
        self._orders = iter(self)

    def generate_order(self) -> lob.OrderRequest:
        return next(self._orders)

    def __iter__(self) -> lob.OrderRequest:
        POSSIBLE_TRADER_IDS: list[str] = [
            "CDEL",
            "GTSC",
            "JETA",
            "NITE",
            "VIRT",
            "GSCO",
            "UBSS",
            "JPMS"
        ]

        PRICE_DIST_MEAN = 100000
        PRICE_DIST_SD = 100

        VOLUME_DIST_MEAN = 10000
        VOLUME_DIST_SD = 1000

        while True:
            # Random walk the distribution mean like the C++ generator, but
            # keep it inside the valid price range so price draws can always
            # produce an in-range tick (the walk would otherwise drift out
            # of bounds and spin forever on the validity retry loop).
            PRICE_DIST_MEAN = self._clamp_rounded(
                round(self.random.gauss(PRICE_DIST_MEAN, PRICE_DIST_SD)),
                MIN_TICKS * TICK, MAX_TICKS * TICK,
            )

            order_request = lob.OrderRequest(
                side=self.random.choice([lob.OrderSide.BUY, lob.OrderSide.SELL]),
                price=self._generate_price(PRICE_DIST_MEAN, PRICE_DIST_SD),
                volume=self._generate_volume(VOLUME_DIST_MEAN, VOLUME_DIST_SD),
                trader_id=lob.Mpid(self.random.choice(POSSIBLE_TRADER_IDS)),
            )

            yield order_request


    

    @staticmethod
    def _clamp_rounded(x: int, lo: float, hi: float) -> int:
        return max(round(lo), min(round(hi), x))

    def _generate_price(self, mean: int, sd: float) -> float:
        while True:
            ticks = round(self.random.gauss(mean, sd) / TICK)
            if MIN_TICKS <= ticks <= MAX_TICKS:
                return _f32(ticks * TICK)

    def _generate_volume(self, mean: int, sd: float) -> int:
        while True:
            volume = round(self.random.gauss(mean, sd))
            if lob.MIN_VOLUME <= volume <= lob.MAX_VOLUME:
                return volume
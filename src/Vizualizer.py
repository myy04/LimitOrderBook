from collections.abc import Callable

import LimitOrderBook as lob
import os

import time

class Vizualizer:
    def __init__(self, pull_from_buffer: Callable[[None], lob.BookSnapshot]):
        self.pull_from_buffer = pull_from_buffer

    def consume_snapshot(self):
        try: 
            snapshot: lob.BookSnapshot = self.pull_from_buffer()
        except Exception as e:
            print(e)
            return

        bids = snapshot.bids
        asks = snapshot.asks
        snapshot_time = snapshot.time

        print(f"last update time: {snapshot_time}")
        print(f"{"TRADER_ID".ljust(20)} | {"PRICE".ljust(20)} | {"VOLUME".ljust(20)}")
        print("-" * 60)

        print("Bids:")
        for bid in bids:
            print(f"{str(bid.trader_id).ljust(20)} | {str(bid.price * lob.PRICE_TICK_SIZE).ljust(20)} | {str(bid.volume).ljust(20)}")

        print("-" * 60)

        print("Asks:")
        for ask in asks:
            print(f"{str(ask.trader_id).ljust(20)} | {str(ask.price * lob.PRICE_TICK_SIZE).ljust(20)} | {str(ask.volume).ljust(20)}")

        time.sleep(1)
        os.system('clear')

    def run(self):
        while True:
            self.consume_snapshot()
    
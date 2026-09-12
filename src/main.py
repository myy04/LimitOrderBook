import time

import LimitOrderBook as lob
from LimitOrderBook import OrderGateway
from LimitOrderBook.OrderGenerator import OrderGenerator


class TradeFeed:
    def __init__(self, history: int = 5):
        self.recent_trades: list[lob.Trade] = []
        self.recent_cancellations: list[lob.SelfTradeCancellation] = []
        self.history = history

        self.trades_total_count = 0
        self.trades_total_volume = 0
        self.cancellations_total_count = 0
        self.cancellation_total_volume = 0

    def process_matches(self, trades, cancellations):
        self.recent_trades = (self.recent_trades + list(trades))[-self.history:]
        self.recent_cancellations = (self.recent_cancellations + list(cancellations))[-self.history:]

        self.trades_total_count += len(trades)
        self.trades_total_volume += sum(trade.volume for trade in trades)

        self.cancellations_total_count += len(cancellations)
        self.cancellation_total_volume += sum(cancel.volume for cancel in cancellations)

    def render(self, snapshot) -> str:
        lines = []

        bids = snapshot.bids
        asks = snapshot.asks

        lines += [f"SNAPSHOT TIME: {snapshot.time}"]
        lines += ["BIDS:"]

        for bid in bids:
            lines += [f"  {str(bid.trader_id)} {bid.side.name} {bid.price:.1f} {bid.volume}"]   

        lines += ["ASKS:"]
        for ask in asks:
            lines += [f"  {str(ask.trader_id)} {ask.side.name} {ask.price:.1f} {ask.volume}"]

        lines += ["STATS:"]
        spread = round(asks[0].price - bids[0].price, 1) if bids and asks else "NA"
        lines += [f"  SPREAD: {spread}"]
        lines += [f"  TOTAL TRADES: {self.trades_total_count}"]
        lines += [f"  TOTAL TRADES VOLUME: {self.trades_total_volume}"]
        lines += [f"  TOTAL CANCELLATIONS: {self.cancellations_total_count}"]
        lines += [f"  TOTAL CANCELLATIONS VOLUME: {self.cancellation_total_volume}"]

        if self.recent_trades:
            lines += [f"RECENT TRADES (last {len(self.recent_trades)}):"]
            for trade in reversed(self.recent_trades):
                lines += [f"  price={trade.price:.1f} volume={trade.volume} aggressor={trade.aggressor_order_id} resting={trade.resting_order_id}"]

        return "\n".join(lines)


def main():
    order_generator = OrderGenerator()
    gateway = OrderGateway()
    feed = TradeFeed()

    try:
        while True:
            order = order_generator.generate_order()
            result = gateway.submit_order(order)
            feed.process_matches(result.trades, result.cancellations)

            print("\033[2J\033[H", end="")
            print(feed.render(gateway.pull_snapshot()))

            time.sleep(0.5)

    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
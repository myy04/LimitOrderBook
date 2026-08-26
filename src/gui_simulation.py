import queue
import threading
import time

import LimitOrderBook as lob
from OrderGenerator import OrderGenerator
from GuiVisualizer import GuiVisualizer


def main() -> None:
    snapshot_queue = queue.Queue(maxsize=50)

    engine = lob.MatchingEngine(push_to_buffer=snapshot_queue.put)
    order_gateway = lob.OrderGateway(engine=engine)

    def order_feed() -> None:
        generator = OrderGenerator(seed=47)
        for order in generator:
            try:
                order_gateway.submit_order_request(order)
            except Exception:
                pass
            time.sleep(0.005)

    feed_thread = threading.Thread(target=order_feed, daemon=True)
    feed_thread.start()

    visualizer = GuiVisualizer(
        pull_from_buffer=snapshot_queue.get,
        max_depth=15,
        max_trades=20,
        refresh_ms=100,
    )
    visualizer.run()


if __name__ == "__main__":
    main()

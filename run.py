from OrderGenerator import OrderGenerator
import LimitOrderBook as lob
from GUI import GUI
import threading

import queue
import sys
from PyQt5.QtWidgets import QApplication

if __name__ == "__main__":
    snapshot_queue = queue.Queue(maxsize=20)
    engine = lob.MatchingEngine(push_to_buffer=snapshot_queue.put_nowait)
    order_gateway = lob.OrderGateway(engine=engine)

    app = QApplication(sys.argv)
    
    gui = GUI(pull_from_buffer=snapshot_queue.get_nowait)

    def feed_orders():
        for order in OrderGenerator():
            try:
                order_gateway.submit_order_request(order)
            except Exception as e:
                pass

    generator_thread = threading.Thread(target = feed_orders, daemon=True)
    generator_thread.start()

    gui.show()
    sys.exit(app.exec_())


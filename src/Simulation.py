from OrderGenerator import OrderGenerator
import LimitOrderBook as lob
from Vizualizer import Vizualizer
import threading

import queue

if __name__ == "__main__":
    snapshot_queue = queue.Queue(maxsize=20)
    engine = lob.MatchingEngine(push_to_buffer=snapshot_queue.put)
    order_gateway = lob.OrderGateway(engine=engine)
    viz_thread = threading.Thread(target = Vizualizer(pull_from_buffer=snapshot_queue.get).run)
    viz_thread.start()
    for order in OrderGenerator():
        try:
            order_gateway.submit_order_request(order) 
        except:
            pass
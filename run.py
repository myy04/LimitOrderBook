from OrderGenerator import OrderGenerator

import LimitOrderBook as lob
from CLI import CLI

import threading

if __name__ == "__main__":
    snapshot_buffer = lob.SnapshotBuffer()
    engine = lob.MatchingEngine(snapshot_buffer=snapshot_buffer)
    order_gateway = lob.OrderGateway(engine=engine)
    cli = CLI(snapshot_buffer=snapshot_buffer)
    cli_thread = threading.Thread(target=cli.run, daemon=True)   
    cli_thread.start()

    for order in OrderGenerator():
        try:
            order_gateway.submit_order_request(order)
        except:
            continue

    cli_thread.join()
import LimitOrderBook as lob

from OrderGenerator import OrderGenerator

import LimitOrderBook as lob
from CLI import CLI

import threading

if __name__ == "__main__":
    engine = lob.MatchingEngine()
    order_gateway = lob.OrderGateway(engine=engine)
    cli = CLI(engine=engine)
    cli_thread = threading.Thread(target=cli.run, daemon=True)   
    cli_thread.start()

    for order in OrderGenerator():
        try:
            order_gateway.submit_order_request(order)
        except lob.OrderException as e:
            continue
        except Exception as e:
            print(e)
            break

    cli_thread.join()
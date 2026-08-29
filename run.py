import threading
import sys, time, os
import LimitOrderBook as lob
from OrderGenerator import OrderGenerator
from CLI import CLI

if __name__ == "__main__":
    arg = str(sys.argv[1])
    engine = lob.cpp.MatchingEngine() if arg == "cpp" else lob.MatchingEngine()

    order_gateway = lob.OrderGateway(engine=engine)
    cli = CLI(engine=engine)
    cli_thread = threading.Thread(target=cli.run, daemon=True)
    cli_thread.start()

    num_trades = 0
    start_time = time.perf_counter()

    try:
        for order in OrderGenerator():
            try:
                result = order_gateway.submit_order_request(order)
                num_trades += len(result.trades) + len(result.cancellations)
            except lob.OrderException:
                continue

    except KeyboardInterrupt:
        print("\nInterrupted by user (Ctrl+C)")
    finally:
        pass

    runtime = time.perf_counter() - start_time
    print("Trades done:", num_trades)
    print("Runtime: ", runtime)
    print("Trades per second:", num_trades / runtime)
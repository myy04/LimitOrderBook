from OrderGenerator import OrderGenerator
import LimitOrderBook as lob

if __name__ == "__main__":
    snapshot_queue: list[lob.BookSnapshot] = []
    engine = lob.MatchingEngine(push_to_buffer=snapshot_queue.append)
    order_gateway = lob.OrderGateway(engine=engine)
    for order in OrderGenerator():
        result: lob.MatchResult = order_gateway.submit_order_request(order) 
        print(snapshot_queue)
        print("-" * 100)    

    
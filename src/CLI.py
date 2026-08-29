import LimitOrderBook as lob
import time, os

class CLI:
    def __init__(self, engine):
        self.engine: lob.MatchingEngine | lob.cpp.MatchingEngine = engine

    def __consume_snapshot(self):
        try:
            snapshot: lob.BookSnapshot = self.engine.pull_snapshot()
        except:
            return
        
        bids = snapshot.bids
        asks = snapshot.asks
        snapshot_time = snapshot.time

        if not bids or not asks: return

        spread = asks[0].price - bids[0].price 
        
        os.system('clear')        

        print("TIME: ", snapshot_time)
        print()

        print("BIDS:\n")
        for trader_id, price, volume in [(x.trader_id, x.price, x.volume) for x in bids]:
            trader_id = str(trader_id).ljust(20)    
            price = str(price * lob.PRICE_TICK_SIZE).ljust(20)
            volume = str(volume).ljust(20)
            
            line = ' | '.join([trader_id, price, volume])
            print(line)
            print('-' * len(line))

        print("ASKS:\n")
        for trader_id, price, volume in [(x.trader_id, x.price, x.volume) for x in asks]:
            trader_id = str(trader_id).ljust(20)    
            price = str(price * lob.PRICE_TICK_SIZE).ljust(20)
            volume = str(volume).ljust(20)
            
            line = ' | '.join([trader_id, price, volume])
            print(line)
            print('-' * len(line))

        print("SPREAD:", spread)
        # time.sleep(1)

    def run(self):
        while True:
            self.__consume_snapshot()


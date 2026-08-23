# Limit Order Book & Matching Engine
A hands-on implementation of a **Limit Order Book (LOB)** built in Python with planned C++ extensions.
---

## 💡 How It Works

1. **Order Ingestion (`OrderGateway`)**:
   Incoming orders are validated for tick sizes, allowed price ranges, and valid quantities. Prices are converted from floating-point decimals to discrete integer ticks (e.g., `$100.50` at tick size `$0.01` becomes `10050`).

2. **Continuous Matching (`MatchingEngine`)**:
   - When an aggressive order arrives, the engine looks at the opposite side of the book (bids for a sell order, asks for a buy order).
   - If the prices cross, trades are executed at the **resting order's price**.
   - If both sides of a trade belong to the same `trader_id`, the engine activates **Self-Trade Prevention (STP)** to cancel the overlapping volume and prevent self-execution.

3. **Resting on Book (`OrderBook`)**:
   Unfilled volume is inserted into the book at its corresponding price level. Orders at the same price are queued in a **Doubly Linked List** to maintain strict **FIFO (First-In, First-Out)** execution order.

---

## 🏗 System Architecture & Complexity

```
Incoming Order
      │
      ▼
┌───────────────────────────────┐
│         OrderGateway          │  ◄── Validates tick sizes & price/volume bounds;
└──────────────┬────────────────┘      assigns sequential order_id & timestamps
               │
               ▼
┌───────────────────────────────┐
│        MatchingEngine         │  ◄── Matches crossing trades & handles STP
└──────────────┬────────────────┘
               │
      ┌────────┴────────┐
      ▼                 ▼
┌───────────┐     ┌───────────┐
│ Bids Tree │     │ Asks Tree │    ◄── SortedDict (Price Levels)
└─────┬─────┘     └─────┬─────┘
      │                 │
      ▼                 ▼
┌─────────────────────────────┐
│      DoublyLinkedList       │    ◄── FIFO Order Queue at each price level
└─────────────────────────────┘
```

### Time Complexity Breakdown

| Operation | Complexity | Implementation Details |
|---|---|---|
| **Peek Best Bid / Ask** | $O(1)$ | Direct lookup of min/max key in `SortedDict` + queue head. |
| **Insert Resting Order** | $O(\log M)$ | Find/insert price level in `SortedDict` ($M$ unique prices) + $O(1)$ list append. |
| **Cancel / Remove Order** | $O(1)$ | $O(1)$ node lookup in a hash map + $O(1)$ node removal in `DoublyLinkedList`. |
| **Match Aggressor Order** | $O(K)$ | Proportional to the number of filled resting orders ($K$). |

---
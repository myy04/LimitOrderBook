from collections.abc import Callable
from collections import deque
import tkinter as tk
from tkinter import ttk
import time

import LimitOrderBook as lob


class GuiVisualizer:
    """Live tkinter visualizer for the limit order book."""

    def __init__(
        self,
        pull_from_buffer: Callable[[], lob.BookSnapshot],
        max_depth: int = 15,
        max_trades: int = 20,
        refresh_ms: int = 100,
    ):
        self.pull_from_buffer = pull_from_buffer
        self.max_depth = max_depth
        self.max_trades = max_trades
        self.refresh_ms = refresh_ms

        self.root = tk.Tk()
        self.root.title("Limit Order Book Live Visualizer")
        self.root.configure(bg="#1e1e1e")

        self._build_ui()

        self.trade_history: deque[str] = deque(maxlen=max_trades)
        self.snapshot_count = 0

    def _build_ui(self) -> None:
        style = ttk.Style(self.root)
        style.theme_use("clam")
        style.configure("TFrame", background="#1e1e1e")
        style.configure(
            "TLabel",
            background="#1e1e1e",
            foreground="#e0e0e0",
            font=("SF Mono", 11),
        )
        style.configure(
            "Header.TLabel",
            font=("SF Mono", 13, "bold"),
            foreground="#ffffff",
        )
        style.configure(
            "Bid.TLabel",
            foreground="#4caf50",
            font=("SF Mono", 11, "bold"),
        )
        style.configure(
            "Ask.TLabel",
            foreground="#f44336",
            font=("SF Mono", 11, "bold"),
        )
        style.configure(
            "MetricValue.TLabel",
            font=("SF Mono", 14, "bold"),
            foreground="#ffffff",
        )

        # Header metrics
        metrics = ttk.Frame(self.root, padding=10)
        metrics.pack(fill=tk.X)

        self.best_bid_label = self._metric_box(metrics, "Best Bid", "—", "Bid.TLabel")
        self.best_ask_label = self._metric_box(metrics, "Best Ask", "—", "Ask.TLabel")
        self.spread_label = self._metric_box(metrics, "Spread", "—")
        self.mid_label = self._metric_box(metrics, "Mid Price", "—")
        self.time_label = self._metric_box(metrics, "Last Update", "—")

        # Book depth
        book_frame = ttk.Frame(self.root, padding=10)
        book_frame.pack(fill=tk.BOTH, expand=True)

        self._build_side(book_frame, side=tk.LEFT, title="Bids", color="#4caf50")
        self._build_side(book_frame, side=tk.RIGHT, title="Asks", color="#f44336")

        # Recent trades
        trades_frame = ttk.Frame(self.root, padding=10)
        trades_frame.pack(fill=tk.X)

        ttk.Label(trades_frame, text="Recent Trades", style="Header.TLabel").pack(anchor=tk.W)
        self.trades_listbox = tk.Listbox(
            trades_frame,
            height=6,
            bg="#252526",
            fg="#e0e0e0",
            font=("SF Mono", 10),
            selectbackground="#333333",
            highlightthickness=0,
            borderwidth=0,
        )
        self.trades_listbox.pack(fill=tk.X, pady=(5, 0))

    def _metric_box(self, parent: ttk.Frame, title: str, initial: str, value_style: str = "MetricValue.TLabel") -> ttk.Label:
        box = ttk.Frame(parent)
        box.pack(side=tk.LEFT, expand=True, fill=tk.X, padx=5)
        ttk.Label(box, text=title).pack()
        value = ttk.Label(box, text=initial, style=value_style)
        value.pack()
        return value

    def _build_side(self, parent: ttk.Frame, side: str, title: str, color: str) -> None:
        frame = ttk.Frame(parent)
        frame.pack(side=side, fill=tk.BOTH, expand=True, padx=5)

        ttk.Label(frame, text=title, style="Header.TLabel").pack(anchor=tk.N)

        headers = ttk.Frame(frame)
        headers.pack(fill=tk.X, pady=(5, 2))
        for col, width, text in [(tk.LEFT, 12, "Price"), (tk.LEFT, 12, "Volume"), (tk.LEFT, 18, "Trader")]:
            ttk.Label(headers, text=text, width=width).pack(side=col)

        canvas = tk.Canvas(
            frame,
            bg="#1e1e1e",
            highlightthickness=0,
            width=350,
            height=self.max_depth * 24,
        )
        canvas.pack(fill=tk.BOTH, expand=True)
        if side == tk.LEFT:
            self.bids_canvas = canvas
        else:
            self.asks_canvas = canvas

    @staticmethod
    def _fmt_price(price: int) -> str:
        return f"{price * lob.PRICE_TICK_SIZE:,.2f}"

    def _draw_rows(
        self,
        canvas: tk.Canvas,
        orders: list[lob.Order],
        color: str,
        align: str,
    ) -> None:
        canvas.delete("all")
        if not orders:
            canvas.create_text(
                175,
                12,
                text="No orders",
                fill="#666666",
                font=("SF Mono", 11),
                anchor=tk.CENTER,
            )
            return

        max_volume = max(order.volume for order in orders) if orders else 1
        row_height = 24
        canvas_width = max(canvas.winfo_width(), 350)

        for i, order in enumerate(orders[: self.max_depth]):
            y = i * row_height
            bar_width = (order.volume / max_volume) * (canvas_width * 0.6)
            x_bar = canvas_width - bar_width if align == "right" else 0

            canvas.create_rectangle(
                x_bar,
                y + 2,
                x_bar + bar_width,
                y + row_height - 2,
                fill=color,
                outline="",
                stipple="gray50",
            )

            x_text = 10 if align == "left" else canvas_width - 10
            anchor = tk.W if align == "left" else tk.E
            canvas.create_text(
                x_text,
                y + row_height // 2,
                text=f"{self._fmt_price(order.price):>12}  {str(order.volume):>12}  {order.trader_id:<18}",
                fill="#ffffff",
                font=("SF Mono", 10),
                anchor=anchor,
            )

    def _update_trades(self, snapshot: lob.BookSnapshot) -> None:
        for trade in snapshot.trades:
            side = "BUY" if trade.aggressor_order.side == lob.OrderSide.BUY else "SELL"
            line = (
                f"{time.strftime('%H:%M:%S', time.localtime(trade.aggressor_order.timestamp))}  "
                f"{side:<4} {trade.volume:>8} @ {self._fmt_price(trade.price):>12}  "
                f"{trade.aggressor_order.trader_id} -> {trade.resting_order.trader_id}"
            )
            self.trade_history.append(line)

        self.trades_listbox.delete(0, tk.END)
        for line in self.trade_history:
            self.trades_listbox.insert(tk.END, line)
        self.trades_listbox.yview(tk.END)

    def consume_snapshot(self) -> None:
        try:
            snapshot: lob.BookSnapshot = self.pull_from_buffer()
        except Exception:
            self.root.after(self.refresh_ms, self.consume_snapshot)
            return

        bids = sorted(snapshot.bids, key=lambda o: o.price, reverse=True)
        asks = sorted(snapshot.asks, key=lambda o: o.price)

        best_bid = bids[0] if bids else None
        best_ask = asks[0] if asks else None

        self.best_bid_label.configure(
            text=self._fmt_price(best_bid.price) if best_bid else "—"
        )
        self.best_ask_label.configure(
            text=self._fmt_price(best_ask.price) if best_ask else "—"
        )

        if best_bid and best_ask:
            spread = (best_ask.price - best_bid.price) * lob.PRICE_TICK_SIZE
            mid = ((best_ask.price + best_bid.price) / 2) * float(lob.PRICE_TICK_SIZE)
            self.spread_label.configure(text=f"{spread:,.2f}")
            self.mid_label.configure(text=f"{mid:,.2f}")
        else:
            self.spread_label.configure(text="—")
            self.mid_label.configure(text="—")

        self.time_label.configure(
            text=time.strftime("%H:%M:%S", time.localtime(snapshot.time))
        )

        self._draw_rows(self.bids_canvas, bids, "#4caf50", "left")
        self._draw_rows(self.asks_canvas, asks, "#f44336", "right")

        if hasattr(snapshot, "trades"):
            self._update_trades(snapshot)

        self.snapshot_count += 1
        self.root.after(self.refresh_ms, self.consume_snapshot)

    def run(self) -> None:
        self.root.after(self.refresh_ms, self.consume_snapshot)
        self.root.mainloop()

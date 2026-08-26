import queue
import numpy as np
import LimitOrderBook as lob
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QColor, QFont, QPalette
from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QSplitter,
    QTableWidget, QTableWidgetItem, QHeaderView, QLabel
)
import pyqtgraph as pg

class GUI(QWidget):
    def __init__(self, pull_from_buffer, parent=None):
        super().__init__(parent)
        self.pull_from_buffer = pull_from_buffer
        self.setWindowTitle("Limit Order Book – Live Snapshot")
        self.resize(1000, 800)

        # ---- Styling palette ----
        self.bid_color = QColor(20, 60, 30)       # dark green
        self.ask_color = QColor(80, 20, 30)       # dark red
        self.bid_text  = QColor(180, 255, 200)
        self.ask_text  = QColor(255, 200, 200)

        pg.setConfigOptions(antialias=True)

        self.history_limit = 100
        self.time_counter = 0
        self.time_history = []
        self.best_bid_history = []
        self.best_ask_history = []

        self._build_ui()

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.consume_snapshot)
        self.timer.start(1000)

        QTimer.singleShot(0, self.consume_snapshot)

    # ------------------------------------------------------------------ #
    #  NEW: Handle window closure cleanly to prevent crash
    # ------------------------------------------------------------------ #
    def closeEvent(self, event):
        # 1. Stop the timer so it doesn't fire while we are destroying widgets
        self.timer.stop()
        
        # 2. Safely clear the pyqtgraph data to avoid C++ segfaults
        try:
            self.bid_curve.clear()
            self.ask_curve.clear()
            self.plot_widget.clear()
        except Exception:
            pass
            
        # 3. Accept the close event and shut down
        event.accept()
        
    # ------------------------------------------------------------------ #
    #  UI construction
    # ------------------------------------------------------------------ #
    def _build_ui(self):
        root = QVBoxLayout(self)
        root.setContentsMargins(12, 12, 12, 12)
        root.setSpacing(8)

        self.status_label = QLabel("Waiting for first snapshot…")
        self.status_label.setStyleSheet(
            "font-size: 14px; font-weight: bold; color: #ddd;"
            "background:#222; padding:6px 10px; border-radius:4px;"
        )
        root.addWidget(self.status_label)

        splitter = QSplitter(Qt.Vertical)
        root.addWidget(splitter, stretch=1)

        self.plot_widget = pg.PlotWidget()
        self.plot_widget.setBackground("#1e1e1e")
        self.plot_widget.setLabel("left", "Price")
        self.plot_widget.setLabel("bottom", "Time (Snapshot Iteration)")
        self.plot_widget.showGrid(x=True, y=True, alpha=0.3)
        self.plot_widget.addLegend(offset=(-10, 10))
        
        self.bid_curve = self.plot_widget.plot([], [], pen=pg.mkPen(color=(0, 255, 100), width=2), name="Best Bid")
        self.ask_curve = self.plot_widget.plot([], [], pen=pg.mkPen(color=(255, 0, 100), width=2), name="Best Ask")
        
        splitter.addWidget(self.plot_widget)

        book_widget = QWidget()
        book_layout = QVBoxLayout(book_widget)
        book_layout.setContentsMargins(0, 0, 0, 0)
        book_layout.setSpacing(4)

        col_header = QLabel(
            f"{'TRADER_ID'.center(22)}|{'PRICE'.center(22)}|{'VOLUME'.center(22)}"
        )
        col_header.setStyleSheet(
            "font-family: monospace; font-size: 12px; font-weight: bold;"
            "color:#333; background:#bbb; padding:4px;"
        )
        col_header.setAlignment(Qt.AlignCenter)
        book_layout.addWidget(col_header)

        self.asks_label = self._section_label("ASKS", self.ask_color, self.ask_text)
        book_layout.addWidget(self.asks_label)

        self.asks_table = self._make_table()
        self._style_table(self.asks_table, self.ask_color, self.ask_text)
        book_layout.addWidget(self.asks_table, stretch=1)

        self.mid_label = QLabel("")
        self.mid_label.setStyleSheet("background:#444; height:2px;")
        self.mid_label.setFixedHeight(2)
        book_layout.addWidget(self.mid_label)

        self.bids_label = self._section_label("BIDS", self.bid_color, self.bid_text)
        book_layout.addWidget(self.bids_label)

        self.bids_table = self._make_table()
        self._style_table(self.bids_table, self.bid_color, self.bid_text)
        book_layout.addWidget(self.bids_table, stretch=1)

        splitter.addWidget(book_widget)
        splitter.setSizes([400, 600])
        self.setStyleSheet("background:#1e1e1e;")

    def _section_label(self, text, bg, fg):
        lbl = QLabel(text)
        lbl.setStyleSheet(
            f"background:{bg.name()}; color:{fg.name()};"
            f"font-weight:bold; font-size:13px; padding:4px 8px;"
        )
        lbl.setAlignment(Qt.AlignCenter)
        return lbl

    def _make_table(self):
        table = QTableWidget(0, 3)
        table.setHorizontalHeaderLabels(["Trader ID", "Price", "Volume"])
        table.verticalHeader().setVisible(False)
        table.setShowGrid(False)
        table.setEditTriggers(QTableWidget.NoEditTriggers)
        table.setSelectionMode(QTableWidget.NoSelection)
        table.setFocusPolicy(Qt.NoFocus)

        header = table.horizontalHeader()
        header.setSectionResizeMode(0, QHeaderView.Stretch)
        header.setSectionResizeMode(1, QHeaderView.Stretch)
        header.setSectionResizeMode(2, QHeaderView.Stretch)

        f = QFont("Monospace", 11)
        table.setFont(f)
        return table

    def _style_table(self, table, bg, fg):
        table.setStyleSheet(f"""
            QTableWidget {{
                background-color: {bg.name()};
                color: {fg.name()};
                gridline-color: rgba(255,255,255,30);
                border: none;
            }}
            QHeaderView::section {{
                background-color: rgba(0,0,0,80);
                color: #f0f0f0;
                font-weight: bold;
                padding: 4px;
                border: none;
            }}
            QTableWidget::item {{
                padding: 4px 8px;
                border-bottom: 1px solid rgba(255,255,255,15);
            }}
        """)

    def _fill_table(self, table, orders, reverse=False):
        table.setRowCount(0)
        items = list(orders)
        if reverse:
            items = list(reversed(items))

        for r, order in enumerate(items):
            table.insertRow(r)
            trader = QTableWidgetItem(str(order.trader_id))
            price  = QTableWidgetItem(f"{order.price * lob.PRICE_TICK_SIZE:.4f}")
            volume = QTableWidgetItem(str(order.volume))

            for c, cell in enumerate((trader, price, volume)):
                cell.setTextAlignment(Qt.AlignCenter)
                cell.setFlags(Qt.ItemIsEnabled)
                table.setItem(r, c, cell)

    def consume_snapshot(self):
        try: 
            snapshot: lob.BookSnapshot = self.pull_from_buffer()
        except queue.Empty:
            return
        except Exception as e:
            self.status_label.setText(f"Error pulling snapshot: {e}")
            return

        bids = snapshot.bids
        asks = snapshot.asks
        snapshot_time = snapshot.time

        self._fill_table(self.asks_table, asks, reverse=True)
        self._fill_table(self.bids_table, bids, reverse=False)

        best_bid = float('nan')
        if bids:
            best_bid = float(max(b.price for b in bids) * lob.PRICE_TICK_SIZE)
            
        best_ask = float('nan')
        if asks:
            best_ask = float(min(a.price for a in asks) * lob.PRICE_TICK_SIZE)

        self.time_history.append(self.time_counter)
        self.best_bid_history.append(best_bid)
        self.best_ask_history.append(best_ask)
        self.time_counter += 1

        if len(self.time_history) > self.history_limit:
            self.time_history.pop(0)
            self.best_bid_history.pop(0)
            self.best_ask_history.pop(0)

        x_data = np.array(self.time_history)
        y_bid = np.array(self.best_bid_history, dtype=float)
        y_ask = np.array(self.best_ask_history, dtype=float)

        self.bid_curve.setData(x_data, y_bid)
        self.ask_curve.setData(x_data, y_ask)

        self.status_label.setText(f"Last update time: {snapshot_time}")
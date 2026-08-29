#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 

#include "./include/lob/Types.h"
#include "./include/lob/OrderBook.h"
#include "./include/lob/MatchingEngine.h"
#include "./include/lob/SnapshotBuffer.h"

namespace py = pybind11;

PYBIND11_MODULE(LimitOrderBook_cpp, m) {
    py::enum_<OrderSide>(m, "OrderSide")
        .value("BUY", OrderSide::BUY)
        .value("SELL", OrderSide::SELL);

    py::class_<Order, std::shared_ptr<Order>>(m, "Order")
        .def(py::init<OrderSide, int, int, int, float, std::string>(), 
            py::arg("side"),
            py::arg("price"),
            py::arg("volume"),
            py::arg("order_id"),
            py::arg("timestamp"),
            py::arg("trader_id")
        )
        .def_readwrite("side", &Order::side)
        .def_readwrite("price", &Order::price)
        .def_readwrite("volume", &Order::volume)
        .def_readwrite("order_id", &Order::order_id)
        .def_readwrite("timestamp", &Order::timestamp)
        .def_readwrite("trader_id", &Order::trader_id);

    py::class_<Trade>(m, "Trade")
        .def(py::init<Order, Order, int, int>(), 
            py::arg("aggressor_order"),
            py::arg("resting_order"),
            py::arg("price"),
            py::arg("volume")
        ) 
        .def_readwrite("aggressor_order", &Trade::aggressor_order)
        .def_readwrite("resting_order", &Trade::resting_order)
        .def_readwrite("price", &Trade::price)
        .def_readwrite("volume", &Trade::volume);


    py::class_<SelfTradeCancellation>(m, "SelfTradeCancellation")
        .def(py::init<Order, Order, int, int>(), 
            py::arg("aggressor_order"),
            py::arg("resting_order"),
            py::arg("price"),
            py::arg("volume")
        ) 
        .def_readwrite("aggressor_order", &SelfTradeCancellation::aggressor_order)
        .def_readwrite("resting_order", &SelfTradeCancellation::resting_order)
        .def_readwrite("price", &SelfTradeCancellation::price)
        .def_readwrite("volume", &SelfTradeCancellation::volume);

    py::class_<MatchResult>(m, "MatchResult")
        .def(py::init<std::vector<Trade>, std::vector<SelfTradeCancellation>>(),
            py::arg("trades"),
            py::arg("cancellations")
        )
        .def_readwrite("trades", &MatchResult::trades)
        .def_readwrite("cancellations", &MatchResult::cancellations);


    py::class_<OrderBook>(m, "OrderBook")
        .def(py::init<>())
        .def("insert_order", &OrderBook::insert_order, py::arg("order"))
        .def("remove_order", &OrderBook::remove_order, py::arg("order"))
        .def("peek_best_bid", &OrderBook::peek_best_bid)
        .def("peek_best_ask", &OrderBook::peek_best_ask);

    py::class_<BookSnapshot>(m, "BookSnapshot")
        .def(py::init<std::vector<Order>, std::vector<Order>, std::string>(),
            py::arg("bids"),
            py::arg("asks"),
            py::arg("time")
        )    
        .def_readwrite("bids", &BookSnapshot::bids)
        .def_readwrite("asks", &BookSnapshot::asks)
        .def_readwrite("time", &BookSnapshot::time);

    // py::class_<SnapshotBuffer>(m, "SnapshotBuffer")
    //     .def(py::init<>())
    //     .def("push", &SnapshotBuffer::push, py::arg("snapshot"))
    //     .def("pull", &SnapshotBuffer::pull);

    py::class_<MatchingEngine>(m, "MatchingEngine")
        .def(py::init<>())
        .def("handle_order", &MatchingEngine::handle_order, py::arg("order"))
        .def("pull_snapshot", &MatchingEngine::pull_snapshot);
};



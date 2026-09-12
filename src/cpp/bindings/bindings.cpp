#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 
#include <sstream> 

#include "../include/lob/Types.h"
#include "../include/lob/OrderBook.h"
#include "../include/lob/MatchingEngine.h"
#include "../include/lob/SnapshotBuffer.h"
#include "../include/lob/OrderGateway.h"

namespace py = pybind11;

PYBIND11_MODULE(LimitOrderBook_cpp, m) {
    py::enum_<OrderSide>(m, "OrderSide")
        .value("BUY", OrderSide::BUY)
        .value("SELL", OrderSide::SELL);

    py::class_<Mpid>(m, "Mpid")
        .def(py::init<const char*>(),
            py::arg("tag")
        )
        .def("__repr__", [](const Mpid& trader_id){
            std::ostringstream oss;
            oss << trader_id;
            return oss.str();
        }) 
        .def_readwrite("tag", &Mpid::tag);


    py::class_<Order>(m, "Order")
        .def(py::init<int, int, int, int, Mpid, OrderSide>(), 
            py::arg("price"),
            py::arg("volume"),
            py::arg("order_id"),
            py::arg("timestamp"),
            py::arg("trader_id"),
            py::arg("side")
        )
        .def_readwrite("price", &Order::price)
        .def_readwrite("volume", &Order::volume)
        .def_readwrite("order_id", &Order::order_id)
        .def_readwrite("timestamp", &Order::timestamp)
        .def_readwrite("trader_id", &Order::trader_id)
        .def_readwrite("side", &Order::side);

    py::class_<Trade>(m, "Trade")
        .def(py::init<int, int, float, int>(), 
            py::arg("aggressor_order_id"),
            py::arg("resting_order_id"),
            py::arg("price"),
            py::arg("volume")
        ) 
        .def_readwrite("aggressor_order_id", &Trade::aggressor_order_id)
        .def_readwrite("resting_order_id", &Trade::resting_order_id)
        .def_readwrite("price", &Trade::price)
        .def_readwrite("volume", &Trade::volume);

    py::class_<SelfTradeCancellation>(m, "SelfTradeCancellation")
        .def(py::init<int, int, float, int>(), 
            py::arg("aggressor_order_id"),
            py::arg("resting_order_id"),
            py::arg("price"),
            py::arg("volume")
        ) 
        .def_readwrite("aggressor_order_id", &SelfTradeCancellation::aggressor_order_id)
        .def_readwrite("resting_order_id", &SelfTradeCancellation::resting_order_id)
        .def_readwrite("price", &SelfTradeCancellation::price)
        .def_readwrite("volume", &SelfTradeCancellation::volume);
        
    py::class_<OrderRequest>(m, "OrderRequest")
        .def(py::init<>())
        .def(py::init<OrderSide, float, size_t, Mpid>(),
            py::arg("side"),
            py::arg("price"),
            py::arg("volume"),
            py::arg("trader_id")
        )
        .def_readwrite("side",      &OrderRequest::side)
        .def_readwrite("price",     &OrderRequest::price)
        .def_readwrite("volume",    &OrderRequest::volume)
        .def_readwrite("trader_id", &OrderRequest::trader_id)
        .def_readwrite("order_id",  &OrderRequest::order_id);

    py::class_<MatchResult>(m, "MatchResult")
        .def(py::init<std::vector<Trade>, std::vector<SelfTradeCancellation>>(),
            py::arg("trades"),
            py::arg("cancellations")
        )
        .def_readwrite("trades", &MatchResult::trades)
        .def_readwrite("cancellations", &MatchResult::cancellations);

    py::class_<BookSnapshot>(m, "BookSnapshot")
        .def(py::init<std::vector<OrderRequest>, std::vector<OrderRequest>, std::string>(),
            py::arg("bids"),
            py::arg("asks"),
            py::arg("time")
        )    
        .def_readwrite("bids", &BookSnapshot::bids)
        .def_readwrite("asks", &BookSnapshot::asks)
        .def_readwrite("time", &BookSnapshot::time);
        
    py::class_<OrderGateway>(m, "OrderGateway")
        .def(py::init<>())                                    
        .def("submit_order", &OrderGateway::submit_order, py::arg("order_request"))
        .def_static("is_order_valid", &OrderGateway::is_order_valid, py::arg("order_request"))
        .def("get_order", &OrderGateway::get_order, py::arg("order_id"),
            py::return_value_policy::reference_internal)
        .def("reset", &OrderGateway::reset)          
        .def("pull_snapshot", &OrderGateway::pull_snapshot);        

    m.attr("MIN_PRICE") = py::cast(
        &CONFIG::MIN_PRICE,
        py::return_value_policy::reference
    );

    m.attr("MAX_PRICE") = py::cast(
        &CONFIG::MAX_PRICE,
        py::return_value_policy::reference
    );

    m.attr("MIN_VOLUME") = py::cast(
        &CONFIG::MIN_VOLUME,
        py::return_value_policy::reference
    );

    m.attr("MAX_VOLUME") = py::cast(
        &CONFIG::MAX_VOLUME,
        py::return_value_policy::reference
    );

    m.attr("PRICE_TICK_SIZE") = py::cast(
        &CONFIG::PRICE_TICK_SIZE,
        py::return_value_policy::reference
    );
};



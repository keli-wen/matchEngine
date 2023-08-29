#include <iostream>
#include "order.h"

namespace UBIEngine {
Order::Order(OrderType type_, OrderSide side_, uint32_t symbol_id_, uint64_t price_, 
    uint64_t quantity_, uint64_t id_)
    : type(type_)
    , side(side_)
    , symbol_id(symbol_id_)
    , price(price_)
    , quantity(quantity_)
    , id(id_) {
    last_executed_price = 0;
    executed_quantity = 0;
    open_quantity = quantity;
    last_executed_price = 0;
    last_executed_quantity = 0;
}

Order Order::newOrder(
    OrderType type, OrderSide side, uint64_t order_id,
    uint32_t symbol_id, uint64_t quantity, uint64_t price) {
    if (type != OrderType::LIMIT && type != OrderType::SBP && type != OrderType::CPBP) {
        if (price != 0 && "Non-limit or Non-SBP or Non-CPBP orders must have a price of 0!")
            throw std::exception();
    }
    // All orders must have positive quantity.
    assert(quantity > 0 && "Orders must have a positive quantity!");
    return Order(type, side, symbol_id, price, quantity, order_id);
}

/******************** For Easy Output. ********************/
static std::string typeToString(OrderType type) {
    switch (type) {
    case OrderType::LIMIT:
        return "LIMIT";
    case OrderType::CPBP:
        return "CPBP";
    case OrderType::SBP:
        return "SBP";
    case OrderType::TOP5_IOC_CANCEL:
        return "TOP5_IOC_CANCEL";
    case OrderType::IOC_CANCEL:
        return "IOC_CANCEL";
    case OrderType::FOK:
        return "FOK";
    default:
        assert(false && "Invalid order type!");
    }
    return "";
}

static std::string sideToString(OrderSide side) {
    switch (side) {
    case OrderSide::Ask:
        return "ASK";
    case OrderSide::Bid:
        return "BID";
    default:
        assert(false && "Invalid order side!");
    }
    return "";
}

std::string Order::toString() const {
    std::string order_string;
    order_string += "Symbol ID: " + std::to_string(symbol_id) + "\n";
    order_string += "Order ID: " + std::to_string(id) + "\n";
    order_string += "Type: " + typeToString(type) + "\n";
    order_string += "Side: " + sideToString(side) + "\n";
    order_string += "Price: " + std::to_string(price) + "\n";
    order_string += "Quantity: " + std::to_string(quantity) + "\n";
    order_string += "Open Quantity: " + std::to_string(open_quantity) + "\n";
    return order_string;
}

std::ostream &operator<<(std::ostream &os, const Order &order) {
    os << order.toString();
    return os;
}

} // namespace UBIEngine
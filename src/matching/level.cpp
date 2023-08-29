#include <iostream>
#include "level.h"

namespace UBIEngine {
Level::Level(uint64_t price_, LevelSide side_, uint32_t symbol_id_)
    : price(price_)
    , side(side_)
    , symbol_id(symbol_id_) {
    volume = 0;
}

const list<Order> &Level::getOrders() const {
    return orders;
}

list<Order> &Level::getOrders() {
    return orders;
}

void Level::addOrder(Order &order) {
    assert(order.isAsk() ? side == LevelSide::Ask : side == LevelSide::Bid
        && "Order is on different side than level!");
    assert(order.getSymbolID() == symbol_id
        && "Order does not have the same symbol ID as the level!");
    volume += order.getOpenQuantity();
    orders.push_back(order);
    VALIDATE_LEVEL;
}

void Level::popFront() {
    assert(!orders.empty() && "Cannot pop from empty level!");
    Order &order_to_remove = orders.front();
    volume -= order_to_remove.getOpenQuantity();
    orders.pop_front();
    VALIDATE_LEVEL;
};

void Level::popBack() {
    assert(!orders.empty() && "Cannot pop from empty level!");
    Order &order_to_remove = orders.back();
    volume -= order_to_remove.getOpenQuantity();
    orders.pop_back();
    VALIDATE_LEVEL;
}

void Level::deleteOrder(const Order &order) {
    volume -= order.getOpenQuantity();
    orders.erase(boost::intrusive::list<Order>::s_iterator_to(order));
    VALIDATE_LEVEL;
}

void Level::reduceVolume(uint64_t amount) {
    assert(volume >= amount &&
        "Reduce level volume greater than its current volume!!!");
    volume -= amount;
    VALIDATE_LEVEL;
}

Order &Level::front() {
    assert(!orders.empty() && "Level is empty!");
    return orders.front();
}

Order &Level::back() {
    assert(!orders.empty() && "Level is empty!");
    return orders.back();
}

std::string Level::toString() const {
    std::string level_string;
    level_string += std::to_string(price) + " X " + std::to_string(volume) + "\n";
    return level_string;
}

std::ostream &operator<<(std::ostream &os, const Level &level) {
    os << level.toString();
    return os;
}

} // namespace UBIEngine
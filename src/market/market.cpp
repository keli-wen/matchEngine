#include "market.h"
#include "map_orderbook.h"

namespace UBIEngine {
OrderBookHandler::OrderBookHandler() {
    id_to_book.clear();
}

void OrderBookHandler::addOrderBook(
    uint32_t symbol_id,
    std::string symbol_name,
    uint64_t previous_close_price,
    uint32_t previous_position)
{
    auto it = id_to_book.find(symbol_id);
    assert(it == id_to_book.end() && "Symbol already exists!");
    id_to_book.insert({symbol_id, std::make_unique<MapOrderBook>(
        symbol_id, previous_close_price, previous_position)});
}

void OrderBookHandler::deleteOrderBook(uint32_t symbol_id, std::string symbol_name)
{
    auto it = id_to_book.find(symbol_id);
    assert(it != id_to_book.end() && "Symbol does not exist!");
    id_to_book.erase(it);
}

void OrderBookHandler::addOrder(const Order &order)
{
    auto it = id_to_book.find(order.getSymbolID());
    assert(it != id_to_book.end() && "Symbol does not exist!");
    MapOrderBook *book = it->second.get();
    book->addOrder(order);
}

void OrderBookHandler::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    auto it = id_to_book.find(symbol_id);
    assert(it != id_to_book.end() && "Symbol does not exist!");
    MapOrderBook *book = it->second.get();
    book->deleteOrder(order_id);
}

void OrderBookHandler::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
{
    auto it = id_to_book.find(symbol_id);
    assert(it != id_to_book.end() && "Symbol does not exist!");
    assert(quantity > 0 && "Quantity must be positive!");
    assert(price > 0 && "Price must be positive!");
    MapOrderBook *book = it->second.get();
    book->executeOrder(order_id, quantity, price);
}

void OrderBookHandler::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    auto it = id_to_book.find(symbol_id);
    assert(it != id_to_book.end() && "Symbol does not exist!");
    assert(order_id > 0 && "Order ID must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    MapOrderBook *book = it->second.get();
    book->executeOrder(order_id, quantity);
}

std::unique_ptr<MapOrderBook> &OrderBookHandler::getOrderBook(uint32_t symbol_id) {
    auto it = id_to_book.find(symbol_id);
    assert(it != id_to_book.end() && "Symbol does not exist!");
    return it->second;
}

std::string OrderBookHandler::toString()
{
    std::string book_handler_string;
    for (const auto &[symbol_id, book_ptr] : id_to_book)
    {
        if (book_ptr)
            book_handler_string += book_ptr->toString() + "\n";
    }
    return book_handler_string;
}

//! Need to note.
Market::Market() {
    orderbook_handler = std::make_unique<OrderBookHandler>();
    id_to_symbol.clear();
    std::cout << "Finish Market constructor" << std::endl;
}

void Market::addSymbol(
    uint32_t symbol_id,
    const std::string &symbol_name,
    uint64_t previous_close_price,
    uint32_t previous_position)
{
    id_to_symbol.insert({symbol_id, std::make_unique<Symbol>(symbol_id, symbol_name)});
    orderbook_handler->addOrderBook(symbol_id, symbol_name,
        previous_close_price, previous_position);
}

void Market::deleteSymbol(uint32_t symbol_id)
{
    auto it = id_to_symbol.find(symbol_id);
    orderbook_handler->deleteOrderBook(symbol_id, it->second->name);
    id_to_symbol.erase(symbol_id);
}

bool Market::hasSymbol(uint32_t symbol_id) const
{
    return id_to_symbol.find(symbol_id) != id_to_symbol.end();
}

void Market::addOrder(const Order &order)
{
    orderbook_handler->addOrder(order);
}

void Market::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    orderbook_handler->deleteOrder(symbol_id, order_id);
}

void Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
{
    orderbook_handler->executeOrder(symbol_id, order_id, quantity, price);
}

void Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    orderbook_handler->executeOrder(symbol_id, order_id, quantity);
}

const uint64_t Market::getBasePrice(uint32_t symbol_id, OrderSide side)
{
    return orderbook_handler->getOrderBook(symbol_id)->getBasePrice(side);
}

const uint64_t Market::getDownLimit(uint32_t symbol_id, OrderSide side)
{
    return orderbook_handler->getOrderBook(symbol_id)->getDownLimit(side);
}

const uint64_t Market::getUpLimit(uint32_t symbol_id, OrderSide side)
{
    return orderbook_handler->getOrderBook(symbol_id)->getUpLimit(side);
}

const PnlHelper& Market::getPnlHelper(uint32_t symbol_id) const {
    return orderbook_handler->getOrderBook(symbol_id)->getPnlHelper();
}

const int64_t Market::calculatePnl(uint32_t symbol_id) const {
    auto& pnl_helper = getPnlHelper(symbol_id);
    uint64_t last_execute_price = orderbook_handler->getOrderBook(symbol_id)->lastTradedPrice();
    return pnl_helper.calculatePnl(last_execute_price);
}

// LCOV_EXCL_START
std::string Market::toString() const
{
    return orderbook_handler->toString();
}

std::ostream &operator<<(std::ostream &os, const Market &market)
{
    os << market.orderbook_handler->toString();
    return os;
}

void Market::dumpMarket(const std::string &name) const
{
    std::ofstream file(name);
    file << *this;
    file.close();
}
// LCOV_EXCL_END
} // namespace UBIEngine
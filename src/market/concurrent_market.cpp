#include "concurrent_market.h"
#include "map_orderbook.h"

namespace UBIEngine {
ConcurrentMarket::ConcurrentMarket(uint8_t num_threads)
    : thread_pool(num_threads)
    , symbol_submission_index(0)
{
    assert(num_threads > 0 && "The number of threads must be positive!");
    orderbook_handlers.reserve(num_threads);
    for (uint8_t i = 0; i < num_threads; ++i)
        orderbook_handlers.push_back(std::make_unique<OrderBookHandler>());
}

void ConcurrentMarket::addSymbol(
    uint32_t symbol_id,
    const std::string &symbol_name,
    uint64_t previous_close_price,
    uint32_t previous_position)
{
    auto it = id_to_symbol.find(symbol_id);
    assert(it == id_to_symbol.end() && "Symbol already exists!");
    id_to_symbol.insert({symbol_id, std::make_unique<Symbol>(symbol_id, symbol_name)});
    id_to_submission_index.insert({symbol_id, symbol_submission_index});
    OrderBookHandler *orderbook_handler = orderbook_handlers[symbol_submission_index].get();
    thread_pool.submitTask(symbol_submission_index,
        [=] { 
            orderbook_handler->addOrderBook(
            symbol_id, symbol_name,
            previous_close_price, previous_position); 
        }
    );
    updateSymbolSubmissionIndex();
}

void ConcurrentMarket::deleteSymbol(uint32_t symbol_id)
{
    auto it = id_to_symbol.find(symbol_id);
    assert(it != id_to_symbol.end() && "Symbol does not exist!");
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[symbol_submission_index].get();
    thread_pool.submitTask(submission_index, [=] { orderbook_handler->deleteOrderBook(symbol_id, it->second->name); });
    id_to_symbol.erase(symbol_id);
    id_to_submission_index.erase(symbol_id);
}

void ConcurrentMarket::addOrder(const Order &order)
{
    uint32_t submission_index = getSubmissionIndex(order.getSymbolID());
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask(submission_index, [=] { orderbook_handler->addOrder(order); });
}

void ConcurrentMarket::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask(submission_index, [=] { orderbook_handler->deleteOrder(symbol_id, order_id); });
}

void ConcurrentMarket::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
{
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask(submission_index, [=] { orderbook_handler->executeOrder(symbol_id, order_id, quantity, price); });
}

void ConcurrentMarket::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask(submission_index, [=] { orderbook_handler->executeOrder(symbol_id, order_id, quantity); });
}

const uint64_t ConcurrentMarket::getBasePrice(uint32_t symbol_id, OrderSide side) {
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    return orderbook_handler->getOrderBook(symbol_id)->getBasePrice(side);
}

const PnlHelper& ConcurrentMarket::getPnlHelper(uint32_t symbol_id) {
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    return orderbook_handler->getOrderBook(symbol_id)->getPnlHelper();
}

uint32_t ConcurrentMarket::getSubmissionIndex(uint32_t symbol_id)
{
    auto it = id_to_submission_index.find(symbol_id);
    return it == id_to_submission_index.end() ? 0 : it->second;
}

void ConcurrentMarket::updateSymbolSubmissionIndex()
{
    symbol_submission_index = (symbol_submission_index + 1) % orderbook_handlers.size();
}

std::string ConcurrentMarket::toString()
{
    std::string market_string;
    std::vector<std::future<std::string>> futures(orderbook_handlers.size());
    for (uint32_t i = 0; i < futures.size(); ++i)
    {
        OrderBookHandler *orderbook_handler = orderbook_handlers[i].get();
        futures[i] = thread_pool.submitWaitableTask(i, [=] { return orderbook_handler->toString(); });
    }
    for (auto &future : futures)
        market_string += future.get();
    return market_string;
}

void ConcurrentMarket::dumpMarket(const std::string &name)
{
    std::ofstream file(name);
    file << *this;
    file.close();
}

std::ostream &operator<<(std::ostream &os, ConcurrentMarket &concurrent_market)
{
    os << concurrent_market.toString();
    return os;
}
} // namespace UBIEngine
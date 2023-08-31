#ifndef UBI_TRADER_CONCURRENT_MARKET_H
#define UBI_TRADER_CONCURRENT_MARKET_H
#include <iostream>
#include <fstream>
#include <memory>
#include "robin_hood.h"
#include "thread_pool.h"
#include "order.h"
#include "orderbook.h"
#include "symbol.h"
#include "market.h"
#include "pnl_helper.h"

namespace UBIEngine {
using namespace Concurrent;
class OrderBookHandler;

class ConcurrentMarket
{
public:
    ConcurrentMarket(ConcurrentMarket &&other) = delete;
    ConcurrentMarket &operator=(ConcurrentMarket &&other) = delete;

    /**
     * A constructor for the concurrent market.
     *
     * @param num_threads the number of worker threads that will be used, require that
     *                    num_threads is positive.
     */
    explicit ConcurrentMarket(uint8_t num_threads = 1);

    /**
     * Adds a new symbol to market asynchronously.
     *
     * @param symbol_id the ID that the symbol is identified by, require that
     *                  the symbol associated with symbol ID does not already exist.
     * @param symbol_name the name of the symbol.
     */
    void addSymbol(
        uint32_t symbol_id,
        const std::string &symbol_name,
        uint64_t previous_close_price = 0,
        uint32_t previous_position = 0);

    /**
     * Removes the symbol from the market asynchronously.
     *
     * @param symbol_id the ID that the symbol is identified by, require that
     *                  the symbol associated with symbol ID exists.
     */
    void deleteSymbol(uint32_t symbol_id);

    /**
     * Submits a new order to the market asynchronously.
     *
     * @param order the order to submit.
     */
    void addOrder(const Order &order);

    /**
     * Deletes an existing order from the market asynchronously, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     */
    void deleteOrder(uint32_t symbol_id, uint64_t order_id);

    /**
     * Executes an existing order in the market asynchronously.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     * @param price the price at which the order is executed, require that price is positive.
     */
    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price);

    /**
     * Executes an existing order in the market asynchronously.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     */
    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity);

    /**
     * @param symbol_id the symbol ID to get base price for.
     * @param side the side to get base price for.
     * @return the base price for the symbol ID and side.
     */
    const uint64_t getBasePrice(uint32_t symbol_id, OrderSide side);

    const PnlHelper& getPnlHelper(uint32_t symbol_id);

    /**
     * @return the string representation of the market.
     */
    [[nodiscard]] std::string toString();

    /**
     * Writes the string representation to a file with the provided
     * name. Creates a new file.
     *
     * @param name the name to the file that will be written to.
     */
    void dumpMarket(const std::string &name);

    friend std::ostream &operator<<(std::ostream &os, ConcurrentMarket &concurrent_market);

private:
    /**
     * Gets the index that that corresponds to the queue in the thread pool
     * that the new task should be submitted to as well the index of the orderbook
     * handler that the task should use.
     *
     * @param symbol_id the symbol ID to get the submission index for
     * @return the submission index associated with the symbol ID.
     */
    uint32_t getSubmissionIndex(uint32_t symbol_id);

    /**
     * Increments the symbol submission index modulo the number of orderbook handlers.
     */
    void updateSymbolSubmissionIndex();

    // The number of orderbook handlers is equivalent to the number of worker threads in the thread pool.
    std::vector<std::unique_ptr<OrderBookHandler>> orderbook_handlers;
    // Maps symbol IDs to symbols.
    robin_hood::unordered_map<uint32_t, std::unique_ptr<Symbol>> id_to_symbol;
    // Maps symbol IDs to the submission indices. A submission index
    // corresponds to the thread pool queue that a task (that is associated with the symbol ID)
    // should be submitted to. Additionally, the submission index corresponds to the orderbook
    // handler that is associated with the symbol ID.
    robin_hood::unordered_map<uint32_t, uint32_t> id_to_submission_index;
    // The thread pool that order operations will be submitted to.
    ThreadPool thread_pool;
    // The index of the thread pool queue and orderbook handler that will be associated
    // with a newly added symbol.
    uint32_t symbol_submission_index;
};
} // namespace UBIEngine
#endif // UBI_TRADER_CONCURRENT_MARKET_H

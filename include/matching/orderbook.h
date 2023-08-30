#ifndef UBI_TRADER_ORDERBOOK_H
#define UBI_TRADER_ORDERBOOK_H
#include "order.h"

namespace UBIEngine {
class OrderBook {
public:
    /**
     * Submits an order to order book.
     *
     * @param order the order to submit.
     */
    virtual void addOrder(Order order) = 0;

    /**
     * Executes an order in the book.
     *
     * @param order_id the ID of the order to execute, require that an order with the provided
     *                 ID exists in the book.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     * @param price the price to execute the order at, require that price is positive.
     */
    virtual void executeOrder(uint64_t order_id, uint64_t quantity, uint64_t price) = 0;

    /**
     * Executes an order in the book.
     *
     * @param order_id the ID of the order to execute, require that an order with the provided
     *                 ID exists in the book.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     */
    virtual void executeOrder(uint64_t order_id, uint64_t quantity) = 0;

    /**
     * Deletes an existing order from the book.
     *
     * @param order_id the ID of the order to delete, require that an order with the provided
     *                 ID exists in the book.
     */
    virtual void deleteOrder(uint64_t order_id) = 0;

    /**
     * @param order_id the ID of the order to check the book for, require that quantity is positive.
     * @return true if the order is in the book and false otherwise.
     */
    [[nodiscard]] virtual bool hasOrder(uint64_t order_id) const = 0;

    /**
     * Retrieves an order from the book.
     *
     * @param order_id the ID of the order being requested, require that an order with
     *                 the provided ID exists in the book.
     * @return the order with provided ID.
     */
    [[nodiscard]] virtual const Order &getOrder(uint64_t order_id) const = 0;

    /**
     * @return true if there are no orders in the book and false otherwise.
     */
    [[nodiscard]] virtual bool empty() const = 0;

    /**
     * @return the symbol ID associated with the book.
     */
    [[nodiscard]] virtual uint32_t getSymbolID() const = 0;

    /**
     * @return the highest bid price if there any bid orders in the book, otherwise zero.
     */
    [[nodiscard]] virtual uint64_t bestBid() const = 0;

    /**
     * @return the lowest ask price if there any ask orders in the book, otherwise max 64-bit integer value.
     */
    [[nodiscard]] virtual uint64_t bestAsk() const = 0;

    /**
     * @return the last traded price if any trades have occurred, otherwise zero.
     */
    [[nodiscard]] virtual uint64_t lastTradedPrice() const = 0;

    /**
     * Writes the string representation of the the orderbook to
     * a file at the provided path. Creates a new file.
     *
     * @param path the path to the file that will be written to.
     */
    virtual void dumpBook(const std::string &path) const = 0;

    /**
     * @return a string representation of the book.
     */
    [[nodiscard]] virtual std::string toString() const = 0;

    virtual ~OrderBook() = default;
};
} // namespace UBIEngine
#endif // UBI_TRADER_ORDERBOOK_H

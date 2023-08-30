#ifndef UBI_TRADER_ORDER_H
#define UBI_TRADER_ORDER_H
#include <string>
#include <iostream>
#include <boost/intrusive/list.hpp>

namespace UBIEngine {
// Only validate order in debug mode.
#ifdef DEBUG
#    define VALIDATE_ORDER validateOrder()
#else
#    define VALIDATE_ORDER
#endif

using namespace boost::intrusive;
class MapOrderBook;
class Level;

/**
 * 支持的订单类型。
 *
 * Limit: 限价申报，该订单类型只会在指定或更优的价格买/卖证券。
 * 对于买入方的限价订单，订单只会在订单价格或更低的价格执行。
 * 对于卖出方的限价订单，订单只会在订单价格或更高的价格执行。
 * 
 * CPBP: 以申报进入主机时集中申报簿中对手方队列的最优价格为其申报价格，
 * 以“卖一”价申报成交（这是有价格的，并不是没有价格）
 * !（如果不能完全成交，则未成交的部分会保留在 OrderBook 中。）
 * 
 * SBP: 以申报进入主机时集中申报簿中本手方队列的最优价格为其申报价格，
 * 以 “买一”价申报成交。注意事项：如果当时本方队列中没有有效申报，交易主机
 * 对“本方最优价格委托”作撤销处理。
 * ! (3.3.6 本方最优价格申报进入交易主机时，集中申报簿中本方无申报的，申报自动撤销。)
 *
 * TOP5_IOC_CANCEL: 最优五档即时成交剩余撤销申报，该订单会尝试立即在当前的最优五档价
 * 格买/卖证券。如果五档内的量不足以完全满足订单，那么未能成交的部分会被撤销。
 * 
 * IOC_CANCEL: 即时成交剩余撤销申报，该订单会尝试立即在当前的最佳价格买/卖证券。
 * 如果市场的量不足以完全满足订单，那么未能成交的部分会被撤销。
 * 
 * FOK: 全额成交或撤销申报，该订单会尝试立即并完全地在当前市场上买/卖证券。
 * 如果市场的量不足以完全满足订单，那么这个订单会被完全撤销，不会有任何部分得到成交。
 */

enum class OrderType
{
    LIMIT = 0,              // 限价申报
    CPBP = 1,               // 对手方最优价格申报 (Counterparty Best Price)
    SBP = 2,                // 本方最优价格申报 (Self Best Price)
    TOP5_IOC_CANCEL = 3,    // 最优五档即时成交剩余撤销申报 (Top 5 Immediate Or Cancel)
    IOC_CANCEL = 4,         // 即时成交剩余撤销申报 (Immediate Or Cancel)
    FOK = 5                 // 全额成交或撤销申报 (Fill Or Kill)
};


/**
 * Order sides.
 *
 * Bid: An order on the bid side is an order to buy a security.
 * Ask: An order on the ask side is an order to sell a security.
 */
enum class OrderSide
{
    // `Bid` 应该是高价优先。
    Bid = 0,
    // `Ask` 应该是低价优先。
    Ask = 1
};


struct Order : public list_base_hook<>
{
public:
    /**
     * Creates a new order.
     *
     * @param order_id the ID of the order, require that order_id is positive.
     * @param symbol_id the symbol ID of the order, require that symbol_id is positive.
     * @param quantity the quantity of the order, require that quantity is positive.
     * @return a new market order.
     */
    static Order newOrder(
        OrderType type, OrderSide side, uint64_t order_id,
        uint32_t symbol_id, uint64_t quantity, uint64_t price=0);

    /**
     * Executes the order.
     *
     * @param price_ the price at which to execute the order, require that price is positive.
     * @param quantity_ the quantity of the order to execute, require that quantity is positive.
     */
    void execute(uint32_t price_, uint64_t quantity_) {
        open_quantity -= quantity_;
        executed_quantity += quantity_;
        last_executed_price = price_;
        last_executed_quantity = quantity_;
        VALIDATE_ORDER;
    }

    /**
     * @return the quantity of the order.
     */
    [[nodiscard]] uint64_t getQuantity() const
    {
        return quantity;
    }

    /**
     * @return the quantity of the order that has been executed.
     */
    [[nodiscard]] uint64_t getExecutedQuantity() const
    {
        return executed_quantity;
    }

    /**
     * @return the quantity of the order that remains to be executed.
     */
    [[nodiscard]] uint64_t getOpenQuantity() const
    {
        return open_quantity;
    }

    /**
     * @return the last executed quantity of the order if the order
     *         has been executed, otherwise zero.
     */
    [[nodiscard]] uint64_t getLastExecutedQuantity() const
    {
        return last_executed_quantity;
    }

    /**
     * @return the ID associated with the order.
     */
    [[nodiscard]] uint64_t getOrderID() const
    {
        return id;
    }

    /**
     * @return the price associated with the order.
     */
    [[nodiscard]] uint64_t getPrice() const
    {
        return price;
    }

    /**
     * @return the side of the order - ask or bid.
     */
    [[nodiscard]] OrderSide getSide() const
    {
        return side;
    }

    /**
     * @return the type of the order - limit, market, stop, stop limit, trailing stop,
     *         or trailing stop limit.
     */
    [[nodiscard]] OrderType getType() const
    {
        return type;
    }

    /**
     * @return the price at which the order was last executed if the order
     *         has been executed, otherwise zero.
     */
    [[nodiscard]] uint64_t getLastExecutedPrice() const
    {
        return last_executed_price;
    }

    /**
     * @return the symbol ID associated with the order.
     */
    [[nodiscard]] uint32_t getSymbolID() const
    {
        return symbol_id;
    }

    /**
     * @return true if order is on the ask side and false otherwise.
     */
    [[nodiscard]] bool isAsk() const
    {
        return side == OrderSide::Ask;
    }

    /**
     * @return true if order is on the bid side and false otherwise.
     */
    [[nodiscard]] bool isBid() const
    {
        return side == OrderSide::Bid;
    }

    /**
     * @return true if the order is a limit order and false otherwise.
     */
    [[nodiscard]] bool isLimit() const
    {
        return type == OrderType::LIMIT;
    }

    /**
     * @return true if the order is a counterparty best price order and false otherwise.
     */
    [[nodiscard]] bool isCPBP() const
    {
        return type == OrderType::CPBP;
    }

    /**
     * @return true if the order is a self best price order and false otherwise.
     */
    [[nodiscard]] bool isSBP() const
    {
        return type == OrderType::SBP;
    }

    /**
     * @return true if the order is a top 5 immediate or cancel order and false otherwise.
     */
    [[nodiscard]] bool isTOP5_IOC_CANCEL() const
    {
        return type == OrderType::TOP5_IOC_CANCEL;
    }

    /**
     * @return true if the order is a immediate or cancel order and false otherwise.
     */
    [[nodiscard]] bool isIOC_CANCEL() const
    {
        return type == OrderType::IOC_CANCEL;
    }

    /**
     * @return true if the order is a full or kill order and false otherwise.
     */
    [[nodiscard]] bool isFOK() const
    {
        return type == OrderType::FOK;
    }

    /**
     * @return true if the order is filled (i.e. the entire quantity of the order
     *         has been executed) and false otherwise.
     */
    [[nodiscard]] bool isFilled() const
    {
        return open_quantity == 0;
    }

    /**
     * Indicates whether two orders are equal. Two orders are equal iff they
     * have the same order ID.
     *
     * @param other another order.
     * @return true if the orders are equal and false otherwise.
     */
    bool operator==(const Order &other) const
    {
        return id == other.id;
    }

    /**
     * Indicates whether two orders are not equal.
     *
     * @param other another order.
     * @return true if the orders are equal and false otherwise.
     */
    bool operator!=(const Order &other) const
    {
        return !(*this == other);
    }

    /**
     * @return the string representation of the order.
     */
    [[nodiscard]] std::string toString() const;

    friend std::ostream &operator<<(std::ostream &os, const Order &order);
    friend class MapOrderBook;
    friend class Level;

private:
    /**
     * A constructor for the Order.
     *
     * @param type_ the type of the order - Limit, Market, Stop, Stop Limit, Trailing Stop, or Trailing Stop Limit.
     * @param side_ the side the order is on - ask or bid.
     * @param time_in_force_ the time in force of the order - FOK, GTC, or IOC.
     * @param symbol_id_ the symbol ID associated with the order.
     * @param price_ the price of the order.
     * @param stop_price_ the stop price of the order.
     * @param trail_amount_ the absolute distance from the market price that trailing stop and trailing
     *                      stop limit orders will trail.
     * @param quantity_ the quantity of the order.
     * @param id_ the ID associated with the order.
     */
    Order(OrderType type_, OrderSide side_, uint32_t symbol_id_,
        uint64_t price_, uint64_t quantity_, uint64_t id_);

    /**
     * Set the price of the order.
     *
     * @param price_ the new price of that order, require that price
     *               is positive if the order is not a market order.
     */
    void setPrice(uint64_t price_)
    {
        price = price_;
        VALIDATE_ORDER;
    }

    /**
     * Set the quantity of the order.
     *
     * @param quantity_ the new quantity of the order, require that quantity_ is positive.
     */
    void setQuantity(uint64_t quantity_)
    {
        quantity = std::min(quantity_, open_quantity);
        open_quantity -= quantity_;
        VALIDATE_ORDER;
    }

    /**
     * Set the ID of the order.
     *
     * @param id_ the new ID of the order.
     */
    void setOrderID(uint64_t id_)
    {
        id = id_;
        VALIDATE_ORDER;
    }

    /**
     * Set the time_in_force of the order.
     *
     * @param action_ the new time_in_force of the order, require that, if type_ is
     *                market or restart, the time_in_force of the order is FOK or IOC.
     */
    void setType(OrderType action_)
    {
        type = action_;
        VALIDATE_ORDER;
    }

    /**
     * Validates the state of the order.
     *
     * @throws Error if any of the following are true: the order
     *               has a non-positive quantity, the order has a non-positive
     *               ID, the order is a market, stop, or trailing stop order that
     *               does not have time in force FOK or IOC, the order is trailing
     *               stop or trailing stop limit order that does not have a positive
     *               trail amount, the order is a stop or stop limit order that does
     *               not have a positive stop price, or the executed quantity / last
     *               executed quantity of the order exceeds the quantity of the order.
     */
    void validateOrder() const;

    OrderType type;                 // 订单类型（例如：限价、市价等）
    OrderSide side;                 // 订单方向（买或卖）
    uint32_t symbol_id;             // 证券的唯一标识
    uint64_t price;                 // 订单的报价（浮点数 * 100）
    uint64_t last_executed_price;   // 上一次成交的价格
    uint64_t id;                    // 订单的唯一标识
    uint64_t quantity;              // 订单的总数量
    uint64_t executed_quantity;     // 已成交的数量
    uint64_t open_quantity;         // 尚未成交的数量
    uint64_t last_executed_quantity;// 上一次成交的数量
};
} // namespace UBIEngine
#endif // UBI_TRADER_ORDER_H

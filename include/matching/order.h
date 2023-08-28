#ifndef UBI_TRADER_ORDER_H
#define UBI_TRADER_ORDER_H
#include <boost/intrusive/list.hpp>

namespace UBIEngine {
// Only validate order in debug mode.
#ifndef NDEBUG
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
 * CPBP: 对手方最优价格申报，该订单会在对手方当前的最优价格买/卖证券。
 * 这种订单可以即时得到满足，但并不能保证得到最佳的成交价格。
 * !（如果不能完全成交，则未成交的部分会保留在 OrderBook 中。）
 * 
 * SBP: 本方最优价格申报，该订单会在自己方向上的当前最优价格买/卖证券。
 * 这种订单会在本方最佳的价格上进行交易。
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
    Bid = 0,
    Ask = 1
};


struct Order : public list_base_hook<>
{
public:

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

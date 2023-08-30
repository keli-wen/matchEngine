#include <iostream>
#include <fstream>
#include "map_orderbook.h"

namespace UBIEngine {
// Only need a symbol ID for the map orderbook.
MapOrderBook::MapOrderBook(uint32_t symbol_id_)
    : symbol_id(symbol_id_)
    , last_traded_price(0)
{}

void MapOrderBook::addOrder(Order order) {
    switch (order.getType()) {
    case OrderType::LIMIT:
        addLimitOrder(order);
        break;
    // 下面的其实都是特殊的 Market Order.
    // 我们可以在里面再进一步调用 addLimitOrder() 来实现.
    case OrderType::CPBP:
    case OrderType::SBP:
    case OrderType::TOP5_IOC_CANCEL:
    case OrderType::IOC_CANCEL:
    case OrderType::FOK:
        addMarketOrder(order);
        break;
    default:
        throw std::runtime_error("Invalid order type!");
        break;
    }
    VALIDATE_ORDERBOOK;
}

/* 执行某个订单，给定价格和数量. */
void MapOrderBook::executeOrder(
    uint64_t order_id,
    uint64_t quantity,
    uint64_t price)
{
    auto orders_it = orders.find(order_id);
    Level &executing_level = orders_it->second.level_it->second;
    Order &executing_order = orders_it->second.order;
    uint64_t executing_quantity =
        std::min(quantity, executing_order.getOpenQuantity());
    executing_order.execute(price, executing_quantity);
    last_traded_price = price;
    executing_level.reduceVolume(
        executing_order.getLastExecutedQuantity()
    );
    if (executing_order.isFilled())
        deleteOrder(order_id, true);
    VALIDATE_ORDERBOOK;
}

/* 执行某个订单，只需要给定数量，因为价格是固定的！ */
void MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity) {
    auto orders_it = orders.find(order_id);
    Level &executing_level = orders_it->second.level_it->second;
    Order &executing_order = orders_it->second.order;
    //! WKL add.
    assert (executing_order.getType() == OrderType::LIMIT
        && "Only limit orders can be executed without give a price!");
    uint64_t executing_quantity =
        std::min(quantity, executing_order.getOpenQuantity());
    uint64_t executing_price = executing_order.getPrice();
    executing_order.execute(executing_price, executing_quantity);
    last_traded_price = executing_price;
    executing_level.reduceVolume(
        executing_order.getLastExecutedQuantity()
    );
    if (executing_order.isFilled())
        deleteOrder(order_id, true);
    VALIDATE_ORDERBOOK;
}

void MapOrderBook::deleteOrder(uint64_t order_id) {
    deleteOrder(order_id, true);
    VALIDATE_ORDERBOOK;
}

/* 删除订单，目前认为所有订单都是用相同的方法，后续可以去掉这个 switch。 */
void MapOrderBook::deleteOrder(uint64_t order_id, bool notification) {
    auto orders_it = orders.find(order_id);
    if (orders_it == orders.end())
        throw std::runtime_error("Order does not exist!");
    auto &levels_it = orders_it->second.level_it;
    Order &deleting_order = orders_it->second.order;
    levels_it->second.deleteOrder(deleting_order);
    if (levels_it->second.empty()) {
        switch (deleting_order.getType()) {
            case OrderType::LIMIT:
            case OrderType::CPBP:
            case OrderType::SBP:
            case OrderType::TOP5_IOC_CANCEL:
            case OrderType::IOC_CANCEL:
            case OrderType::FOK:
                deleting_order.isAsk()
                    ? ask_levels.erase(levels_it)
                    : bid_levels.erase(levels_it);
                break;
            default:
                assert(false && "Invalid order type!");
        }
    }
    orders.erase(orders_it);
}

void MapOrderBook::addLimitOrder(Order &order) {
    //TODO: 可能需要 check 基准价格是否合法。
    match(order);
    if (!order.isFilled())
        insertLimitOrder(order);
}

/* 如果 match 没有 match 上或者并没有全部执行就需要 insert 到 Level 中。 */
void MapOrderBook::insertLimitOrder(const Order &order) {
    if (order.isAsk()) {
        // `emplace_hint` @arg1 是一个 iterator，表示可能的插入位置，用来
        // 提高插入效率。
        auto level_it = ask_levels.emplace_hint(ask_levels.begin(),
            std::piecewise_construct, std::make_tuple(order.getPrice()),
            std::make_tuple(order.getPrice(), LevelSide::Ask, symbol_id));
        auto [orders_it, success] = orders.emplace(order.getOrderID(),
            OrderWrapper{order, level_it});
        level_it->second.addOrder(orders_it->second.order);
    }
    else {
        auto level_it = bid_levels.emplace_hint(bid_levels.end(),
            std::piecewise_construct, std::make_tuple(order.getPrice()),
            std::make_tuple(order.getPrice(), LevelSide::Bid, symbol_id));
        auto [orders_it, success] = orders.emplace(order.getOrderID(),
            OrderWrapper{order, level_it});
        level_it->second.addOrder(orders_it->second.order);
    }
}

void MapOrderBook::addMarketOrder(Order &order) {
    //TODO: 可能需要 check 基准价格是否合法。
    if (order.getType() == OrderType::CPBP) {
        // 如果是对手方最优价格且对方盘口为空，则直接 skip。
        if (order.isAsk() && bid_levels.empty())
            return void();
        if (order.isBid() && ask_levels.empty())
            return void();
        // 直接转化为 对手最优价格的 LIMIT Order.
        order.setType(OrderType::LIMIT);
        order.setPrice(order.isAsk() ? bestBid() : bestAsk());
        addLimitOrder(order);
    }
    else if (order.getType() == OrderType::SBP) {
        // 如果是己方最优价格且己方盘口为空，则直接 skip。
        if (order.isAsk() && ask_levels.empty())
            return void();
        if (order.isBid() && bid_levels.empty())
            return void();
        // 直接转化为 己方最优价格的 LIMIT Order.
        order.setType(OrderType::LIMIT);
        order.setPrice(order.isAsk() ? bestAsk() : bestBid());
        addLimitOrder(order);
    }
    else if (order.getType() == OrderType::TOP5_IOC_CANCEL) {
        uint64_t fifth_price = 0;
        if (order.isAsk()) {
            if (bid_levels.size() < 5) {
                if (!bid_levels.empty()) {
                    // 最差的 bid 的价格
                    fifth_price = bid_levels.begin()->first;
                }
            } else {
                auto it = bid_levels.rbegin();
                std::advance(it, 4);
                fifth_price = it->first;
            }
        } else {
            if (ask_levels.size() < 5) {
                if (!ask_levels.empty()) {
                    // 最差的 ask 价格
                    fifth_price = ask_levels.rbegin()->first;
                }
            } else {
                auto it = ask_levels.begin();
                std::advance(it, 4);
                fifth_price = it->first;
            }
        }
        
        // 如果价格层次为空（即没有合适的买/卖单），则可能不需要处理订单，
        // 或者需要采取其他策略。
        if (fifth_price == 0)
            return void();

        order.setPrice(fifth_price);
        match(order);
    }
    else {
        // 剩下的 IOC_CANCEL 和 FOK 都是纯正的 Market Order.
        order.setPrice(order.isAsk() ?
            0 : std::numeric_limits<uint64_t>::max());
        match(order);
    }
}

void MapOrderBook::match(Order &order) {
    // Order is a FOK order that cannot be filled.
    if (order.isFOK() && !canMatchOrder(order))
        return;
    if (order.isAsk()) {
        auto bid_levels_it = bid_levels.rbegin();
        Order &ask_order = order;
        while (bid_levels_it != bid_levels.rend()
               && bid_levels_it->first >= ask_order.getPrice()
               && !ask_order.isFilled())
        {
            Level &bid_level = bid_levels_it->second;
            Order &bid_order = bid_level.front();
            uint64_t executing_price = bid_order.getPrice();
            executeOrders(ask_order, bid_order, executing_price);
            bid_level.reduceVolume(bid_order.getLastExecutedQuantity());
            if (bid_order.isFilled())
                deleteOrder(bid_order.getOrderID(), true);
            // ! Reset the level iterator - iterator may be invalidated
            // ! if the order is deleted.
            // ! 由于我们不交易完当前的 level 就不会去下一个 level，所以不需要
            // ! increment iterator.
            bid_levels_it = bid_levels.rbegin();
        }
    }
    if (order.isBid()) {
        auto ask_levels_it = ask_levels.begin();
        Order &bid_order = order;
        while (ask_levels_it != ask_levels.end()
               && ask_levels_it->first <= bid_order.getPrice()
               && !bid_order.isFilled())
        {
            Level &ask_level = ask_levels_it->second;
            Order &ask_order = ask_level.front();
            uint64_t executing_price = ask_order.getPrice();
            executeOrders(ask_order, bid_order, executing_price);
            ask_level.reduceVolume(ask_order.getLastExecutedQuantity());
            if (ask_order.isFilled())
                deleteOrder(ask_order.getOrderID(), true);
            // ! Reset the level iterator - iterator may be invalidated
            // ! if the order is deleted.
            ask_levels_it = ask_levels.begin();
        }
    }
}

void MapOrderBook::executeOrders(
    Order &ask,
    Order &bid,
    uint64_t executing_price)
{
    // Calculate the minimum quantity to match.
    uint64_t matched_quantity =
        std::min(ask.getOpenQuantity(), bid.getOpenQuantity());
    bid.execute(executing_price, matched_quantity);
    ask.execute(executing_price, matched_quantity);
    last_traded_price = executing_price;
}

/* 用于服务 TYPE 为 FOK 的订单 */
bool MapOrderBook::canMatchOrder(const Order &order) const {
    uint64_t price = order.getPrice();
    uint64_t quantity_required = order.getOpenQuantity();
    uint64_t quantity_available = 0;
    if (order.isAsk()) {
        auto bid_levels_it = bid_levels.rbegin();
        while (bid_levels_it != bid_levels.rend()
               && bid_levels_it->first >= price)
        {
            uint64_t level_volume = bid_levels_it->second.getVolume();
            uint64_t quantity_needed =
                quantity_required - quantity_available;
            quantity_available += std::min(level_volume, quantity_needed);
            if (quantity_available >= quantity_required)
                return true;

            ++ bid_levels_it;
        }
    }
    else {
        auto ask_levels_it = ask_levels.begin();
        while (ask_levels_it != ask_levels.end()
               && ask_levels_it->first <= price)
        {
            uint64_t level_volume = ask_levels_it->second.getVolume();
            uint64_t quantity_needed =
                quantity_required - quantity_available;
            quantity_available += std::min(level_volume, quantity_needed);
            if (quantity_available >= quantity_required)
                return true;

            ++ ask_levels_it;
        }
    }
    return false;
}

std::string MapOrderBook::toString() const {
    std::string book_string;
    book_string += "SYMBOL ID : " + std::to_string(symbol_id) + "\n";
    book_string +=
        "LAST TRADED PRICE: " + std::to_string(last_traded_price) + "\n";
    book_string += "BID ORDERS\n";
    for (const auto &[price, level] : bid_levels)
        book_string += level.toString();
    book_string += "ASK ORDERS\n";
    for (const auto &[price, level] : ask_levels)
        book_string += level.toString();
    return book_string;
}

void MapOrderBook::dumpBook(const std::string &path) const {
    std::ofstream file(path);
    file << toString();
    file.close();
}

std::ostream &operator<<(
    std::ostream &os,
    const MapOrderBook &book)
{
    os << book.toString();
    return os;
}

void MapOrderBook::validateOrderBook() const {
    validateLimitOrders();
}

void MapOrderBook::validateLimitOrders() const {
    // uint64_t current_best_ask = ask_levels.empty() ? std::numeric_limits<uint64_t>::max() : ask_levels.begin()->first;
    // uint64_t current_best_bid = bid_levels.empty() ? 0 : bid_levels.rbegin()->first;
    // assert(current_best_ask > current_best_bid && "Best bid price should never be lower than best ask price!");
    assert(bestAsk() > bestBid()
        && "Best bid price should never be lower than best ask price!");

    for (const auto &[price, level] : ask_levels) {
        assert(!level.empty() && "Empty limit levels should never be in the orderbook!");
        assert(level.getPrice() == price && "Limit level price should have same value as map key!");
        assert(level.getSide() == LevelSide::Ask && "Limit level with bid side cannot be on the ask side of the book!");
        const auto &level_orders = level.getOrders();
        for (const auto &order : level_orders) {
            assert(!order.isFilled() && "Limit level should not contain any filled orders!");
            assert(order.getType() == OrderType::LIMIT && "Limit level contains order that is not a limit order!");
        }
    }

    for (const auto &[price, level] : bid_levels) {
        assert(!level.empty() && "Empty limit levels should never be in the orderbook!");
        assert(level.getPrice() == price && "Limit level price should have same value as map key!");
        assert(level.getSide() == LevelSide::Bid && "Limit level with ask side cannot be on the bid side of the book!");
        const auto &level_orders = level.getOrders();
        for (const auto &order : level_orders) {
            assert(!order.isFilled() && "Limit level should not contain any filled orders!");
            assert(order.getType() == OrderType::LIMIT && "Limit level contains order that is not a limit order!");
        }
    }
}
} // namespace UBIEngine
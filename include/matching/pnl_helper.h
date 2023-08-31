#ifndef UBI_TRADER_PNL_HELPER_H
#define UBI_TRADER_PNL_HELPER_H
#include "symbol.h"
#include "order.h"

namespace UBIEngine {

class PnlHelper {
public:
    PnlHelper(uint64_t prev_close_price_ = 0, uint32_t prev_position_ = 0);

    /**
     * @param current_price the current price of the security.
     * @return the PnL of the position. [int64_t]
     */
    const int64_t calculatePnl(uint64_t current_price) const;

    /**
     * Updates the account based on executed orders.
     *
     * @param direction the direction of the order (Bid or Ask)
     * @param executed_price the price at which the order was executed.
     * @param executed_qty the quantity of the order that was executed.
     */
    void updateAccount(OrderSide side,
        uint64_t executed_price, uint32_t executed_quantity);

    [[nodiscard]] uint64_t getPreviousClosePrice() const {
        return prev_close_price;
    }

    [[nodiscard]] int64_t getCash() const {
        return cash;
    }

    [[nodiscard]] uint32_t getPosition() const {
        return position;
    }

    [[nodiscard]] uint32_t getPrevPosition() const {
        return prev_position;
    }

    [[nodiscard]] std::string toString() const;

    friend std::ostream &operator<<(std::ostream &os, const Order &order);
private:

    // 维护昨日收盘价
    uint64_t prev_close_price;
    // 维护当前持仓
    uint32_t position;
    // 维护今日开盘持仓
    uint32_t prev_position;
    // 维护现金账户
    int64_t cash;
};
}
#endif
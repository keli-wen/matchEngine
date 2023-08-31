#include "pnl_helper.h"

namespace UBIEngine {
PnlHelper::PnlHelper(
    uint64_t prev_close_price_,
    uint32_t prev_position_)
    : prev_close_price(prev_close_price_)
    , position(prev_position_)
    , prev_position(prev_position_)
    , cash(0) {}

int64_t PnlHelper::calculatePnl(uint64_t current_price) {
    int64_t current_valuation =
        static_cast<int64_t>(position) * current_price;
    int64_t prev_valuation =
        static_cast<int64_t>(prev_position) * prev_close_price;
    
    return current_valuation - prev_valuation + cash;
}

void PnlHelper::updateAccount(OrderSide side,
    uint64_t executed_price, uint32_t executed_quantity)
{
    if (side == OrderSide::Bid) {
        position += executed_quantity;
        cash -=
            static_cast<int64_t>(executed_price) * executed_quantity;
    } else {
        assert (position >= executed_quantity &&
            "Cannot sell more than you have!");
        position -= executed_quantity;
        cash +=
            static_cast<int64_t>(executed_price) * executed_quantity;
    }
}

std::string PnlHelper::toString() const {
    std::string pnl_helper_string;
    pnl_helper_string += "Previous Close Price: " + std::to_string(prev_close_price) + "\n";
    pnl_helper_string += "Position: " + std::to_string(position) + "\n";
    pnl_helper_string += "Previous Position: " + std::to_string(prev_position) + "\n";
    pnl_helper_string += "Cash: " + std::to_string(cash) + "\n";
    return pnl_helper_string;
}

std::ostream &operator<<(std::ostream &os, const PnlHelper &pnl_helper) {
    os << pnl_helper.toString();
    return os;
}

} // namespace UBIEngine
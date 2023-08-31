#ifndef UBI_TRADER_UTILS_SORT_H
#define UBI_TRADER_UTILS_SORT_H
#include "io/type.h"
#include <algorithm>
#include <thread>
#include <cstring>

namespace UBIEngine::Utils {
// Comparator for twap_order.
bool my_compare_twap(const IO::twap_order &a, const IO::twap_order &b) {
    if (a.timestamp != b.timestamp)
        return a.timestamp < b.timestamp;
    return std::strncmp(a.instrument_id, b.instrument_id, 8) < 0;
}

bool my_compare_pnl(const IO::pnland_pos &a, const IO::pnland_pos &b) {
    return std::strncmp(a.instrument_id, b.instrument_id, 8) < 0;
}

// Multi-threaded sort function.
template <typename T, typename Comparator = std::less<T>>
void multiThreadSort(std::vector<T> &ans, Comparator comp = Comparator()) {
    auto middle = ans.begin() + ans.size() / 2;

    std::thread firstHalf([&]() {
        std::sort(ans.begin(), middle, comp);
    });
    std::thread secondHalf([&]() {
        std::sort(middle, ans.end(), comp);
    });

    firstHalf.join();
    secondHalf.join();

    std::inplace_merge(ans.begin(), middle, ans.end(), comp);
}


} // namespace UBIEngine::Utils
#endif // UBI_TRADER_UTILS_SORT_H
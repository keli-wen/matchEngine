#ifndef UBI_TRADER_SYMBOL_H
#define UBI_TRADER_SYMBOL_H
#include <iostream>
#include <cstdint>

namespace UBIEngine {
/**
 * Represents a publicly traded security.
 */
struct Symbol
{
    // A positive number used to identify the security.
    uint32_t id;
    // A unique series of characters and or numbers used to identify the security.
    std::string name;
    Symbol(uint32_t id_, std::string name_);
    friend std::ostream &operator<<(std::ostream &os, const Symbol &symbol);
};
} // namespace UBIEngine
#endif // UBI_TRADER_SYMBOL_H

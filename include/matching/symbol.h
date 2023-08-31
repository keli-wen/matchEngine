#ifndef UBI_TRADER_SYMBOL_H
#define UBI_TRADER_SYMBOL_H
#include <iostream>
#include <cstdint>
#include "robin_hood.h"

namespace UBIEngine {
/**
 * Represents a publicly traded security.
 */
struct Symbol {
    // A positive number used to identify the security.
    uint32_t id;
    // A unique series of characters and or numbers used to identify the security.
    std::string name;
    Symbol(uint32_t id_, std::string name_);
    friend std::ostream &operator<<(std::ostream &os, const Symbol &symbol);
};

class SymbolManager {
public:
    SymbolManager();
    SymbolManager(SymbolManager &&other) = delete;
    SymbolManager &operator=(SymbolManager &&other) = delete;

    /**
     * Returns the symbol ID associated with the instrument ID.
     *
     * @param instrument_id the instrument ID associated with the symbol ID.
     * @return the symbol ID associated with the instrument ID.
     */
    uint32_t getSymbolId(const char instrument_id[8]);

    /**
     * Returns the instrument Name associated with the symbol ID.
     *
     * @param symbol_id the symbol ID associated with the instrument ID.
     * @return the instrument Name associated with the symbol ID.
     */
    std::string getInstrumentName(uint32_t symbol_id);

    [[nodiscard]] uint32_t getCounter() const { return counter; }
private:
    // 使用 std::unordered_map 维护一个从 instrument_id 到 symbol_id 的映射
    robin_hood::unordered_map<std::string, uint32_t> id_to_symbol_map;

    // 使用另一个映射从 symbol_id 回到 instrument_id
    robin_hood::unordered_map<uint32_t, std::string> symbol_to_id_map;

    // 一个简单的计数器为每个新的 instrument_id 分配一个新的 symbol_id
    uint32_t counter = 0;
};
} // namespace UBIEngine
#endif // UBI_TRADER_SYMBOL_H

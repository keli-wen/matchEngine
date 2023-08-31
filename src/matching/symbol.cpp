#include <algorithm>
#include <string>
#include "symbol.h"
namespace UBIEngine {
Symbol::Symbol(uint32_t id_, std::string name_)
    : id(id_)
    , name(std::move(name_)) {}

std::ostream &operator<<(std::ostream &os, const Symbol &symbol) {
    os << "Symbol Name: " << symbol.name
       << "Symbol ID: " << symbol.id << std::endl;
    return os;
}

SymbolManager::SymbolManager(): counter(0) {}

uint32_t SymbolManager::getSymbolId(const char instrument_id[8]) {
    //! 这里用 8 会产生一些 undefine 的字符。
    std::string id_str(instrument_id);
    auto it = id_to_symbol_map.find(id_str);
    if (it != id_to_symbol_map.end()) {
        return it->second;
    }

    uint32_t new_symbol_id = counter++;
    id_to_symbol_map[id_str] = new_symbol_id;
    symbol_to_id_map[new_symbol_id] = id_str;

    return new_symbol_id;
}

// 从 symbol_id 获取 instrument_id
std::string SymbolManager::getInstrumentName(uint32_t symbol_id) {
    auto it = symbol_to_id_map.find(symbol_id);
    if (it != symbol_to_id_map.end()) {
        return it->second;
    }

    // 如果不存在，则返回一个空字符串
    return "";
}
} // namespace UBIEngine
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
} // namespace UBIEngine
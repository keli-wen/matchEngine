#include <gtest/gtest.h>
#include "symbol.h"

using namespace UBIEngine;

TEST(SymbolTest, initSymbol) {
    Symbol symbol(1, "AAPL");
    EXPECT_EQ(symbol.id, 1);
    EXPECT_EQ(symbol.name, "AAPL");
}

TEST(SymbolManagerTest, initSymbolManager) {
    SymbolManager symbol_manager;
    EXPECT_EQ(symbol_manager.getCounter(), 0);
}

TEST(SymbolManagerTest, getSymbolId) {
    SymbolManager symbol_manager;
    EXPECT_EQ(symbol_manager.getCounter(), 0);

    uint32_t symbol_id = symbol_manager.getSymbolId("AAPL");
    EXPECT_EQ(symbol_id, 0);
    uint32_t symbol_id2 = symbol_manager.getSymbolId("AMZN");
    EXPECT_EQ(symbol_id2, 1);
    uint32_t symbol_id3 = symbol_manager.getSymbolId("AAPL");
    EXPECT_EQ(symbol_id3, 0);
}

TEST(SymbolManager, getInstrumentName) {
    SymbolManager symbol_manager;
    EXPECT_EQ(symbol_manager.getCounter(), 0);

    uint32_t symbol_id = symbol_manager.getSymbolId("AAPL");
    EXPECT_EQ(symbol_id, 0);
    uint32_t symbol_id2 = symbol_manager.getSymbolId("AMZN");
    EXPECT_EQ(symbol_id2, 1);
    uint32_t symbol_id3 = symbol_manager.getSymbolId("AAPL");
    EXPECT_EQ(symbol_id3, 0);

    std::string instrument_name = symbol_manager.getInstrumentName(0);
    EXPECT_EQ(instrument_name, "AAPL");
    std::string instrument_name2 = symbol_manager.getInstrumentName(1);
    EXPECT_EQ(instrument_name2, "AMZN");
    std::string instrument_name3 = symbol_manager.getInstrumentName(2);
    EXPECT_EQ(instrument_name3, "");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

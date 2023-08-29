#include <gtest/gtest.h>
#include "symbol.h"

using namespace UBIEngine;

TEST(SymbolTest, initSymbol) {
    Symbol symbol(1, "AAPL");
    EXPECT_EQ(symbol.id, 1);
    EXPECT_EQ(symbol.name, "AAPL");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

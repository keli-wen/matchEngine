#include <gtest/gtest.h>
#include "order.h"

using namespace UBIEngine;

TEST(OrderTest, newOrder) {
    uint32_t symbol1 = 1;
    uint32_t quantity1 = 100;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    Order order1 = Order::newOrder(OrderType::LIMIT,
        OrderSide::Bid, id1, symbol1, quantity1, price1, false);
    EXPECT_EQ(order1.getOrderID(), id1);
    EXPECT_EQ(order1.getSymbolID(), symbol1);
    EXPECT_EQ(order1.getPrice(), price1);
    EXPECT_EQ(order1.getQuantity(), quantity1);
    EXPECT_EQ(order1.getSide(), OrderSide::Bid);
    EXPECT_EQ(order1.getType(), OrderType::LIMIT);
    EXPECT_TRUE(order1.isLimit());
    EXPECT_FALSE(order1.isCPBP() || order1.isSBP() || order1.isFOK()
        || order1.isIOC_CANCEL() || order1.isTOP5_IOC_CANCEL());
    EXPECT_EQ(order1.getLastExecutedPrice(), 0);
    EXPECT_EQ(order1.getExecutedQuantity(), 0);
    EXPECT_EQ(order1.getOpenQuantity(), quantity1);
    EXPECT_FALSE(order1.isStrategyOrder());

    // Check if the order is a strategy order. [true]
    Order order2 = Order::newOrder(OrderType::LIMIT,
        OrderSide::Bid, id1, symbol1, quantity1, price1, true);
    EXPECT_TRUE(order2.isStrategyOrder());
}

TEST (OrderTest, newOrder_assert) {
    uint32_t symbol1 = 1;
    uint32_t quantity1 = 100;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    EXPECT_THROW(
        Order::newOrder(OrderType::FOK, OrderSide::Bid, id1, symbol1, quantity1, price1, false),
        std::exception
    );
}

TEST (OrderTest, executeOrder) {
    uint32_t symbol1 = 1;
    uint32_t quantity1 = 100;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    Order order1 = Order::newOrder(OrderType::LIMIT,
        OrderSide::Bid, id1, symbol1, quantity1, price1, false);
    
    uint32_t exe_price1 = 98;
    uint32_t exe_quantity1 = 50;
    order1.execute(exe_price1, exe_quantity1);
    EXPECT_EQ(order1.getExecutedQuantity(), exe_quantity1);
    EXPECT_EQ(order1.getOpenQuantity(), quantity1 - exe_quantity1);
    EXPECT_EQ(order1.getLastExecutedPrice(), exe_price1);
    EXPECT_EQ(order1.getLastExecutedQuantity(), exe_quantity1);

    uint32_t exe_price2 = 99;
    uint32_t exe_quantity2 = 50;
    order1.execute(exe_price2, exe_quantity2);
    EXPECT_EQ(order1.getExecutedQuantity(),
        exe_quantity1 + exe_quantity2);
    EXPECT_EQ(order1.getOpenQuantity(),
        quantity1 - exe_quantity1 - exe_quantity2);
    EXPECT_EQ(order1.getLastExecutedPrice(), exe_price2);
    EXPECT_EQ(order1.getLastExecutedQuantity(), exe_quantity2);
    // Assert that the order is filled.
    EXPECT_TRUE(order1.isFilled());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

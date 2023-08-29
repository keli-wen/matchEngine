#include <gtest/gtest.h>
#include "order.h"

using namespace UBIEngine;

TEST(OrderTest, newOrder) {
    uint32_t symbol1 = 1;
    uint32_t quantity1 = 100;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    Order order1 = Order::newOrder(OrderType::LIMIT, OrderSide::Bid, id1, symbol1, quantity1, price1);
    EXPECT_EQ(order1.getOrderID(), id1);
    EXPECT_EQ(order1.getSymbolID(), symbol1);
    EXPECT_EQ(order1.getPrice(), price1);
    EXPECT_EQ(order1.getQuantity(), quantity1);
    EXPECT_EQ(order1.getSide(), OrderSide::Bid);
    EXPECT_EQ(order1.getType(), OrderType::LIMIT);
    EXPECT_EQ(order1.getLastExecutedPrice(), 0);
    EXPECT_EQ(order1.getExecutedQuantity(), 0);
    EXPECT_EQ(order1.getOpenQuantity(), quantity1);
}

TEST (OrderTest, newOrder_assert) {
    uint32_t symbol1 = 1;
    uint32_t quantity1 = 100;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    EXPECT_THROW(
        Order::newOrder(OrderType::FOK, OrderSide::Bid, id1, symbol1, quantity1, price1),
        std::exception
    );
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

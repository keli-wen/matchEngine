#include <gtest/gtest.h>
#include "level.h"

using namespace UBIEngine;

TEST(LevelTest, initLevel) {
    uint32_t symbol1 = 1;
    uint64_t price1 = 100;
    LevelSide side1 = LevelSide::Bid;
    Level level1 = Level(price1, side1, symbol1);
    EXPECT_EQ(level1.getPrice(), price1);
    EXPECT_EQ(level1.getVolume(), 0);
    EXPECT_EQ(level1.getOrders().size(), 0);
}

TEST(LevelTest, testPushAndGet) {
    uint32_t symbol1 = 1;
    uint32_t quantity1 = 100;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    Order order1 = Order::newOrder(OrderType::LIMIT,
        OrderSide::Bid, id1, symbol1, quantity1, price1);

    Level level1 = Level(price1, LevelSide::Bid, symbol1);
    level1.addOrder(order1);
    EXPECT_EQ(level1.getOrders().size(), 1);
    EXPECT_EQ(level1.getVolume(), 100);
    EXPECT_EQ(level1.getOrders().front().getOrderID(), 1);
    EXPECT_EQ(level1.getOrders().front().getOpenQuantity(), quantity1);
    EXPECT_EQ(level1.getOrders().front().getSymbolID(), 1);
    EXPECT_EQ(level1.getOrders().front().getPrice(), 100);
    EXPECT_EQ(level1.getOrders().front().getSide(), OrderSide::Bid);
    EXPECT_EQ(level1.getOrders().front().getType(), OrderType::LIMIT);
}

TEST(LevelTest, testPop) {
    uint32_t symbol1 = 1;
    uint32_t quantity1 = 100;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    Order order1 = Order::newOrder(OrderType::LIMIT,
        OrderSide::Bid, id1, symbol1, quantity1, price1);

    Level level1 = Level(price1, LevelSide::Bid, symbol1);
    level1.addOrder(order1);

    // Test pop back.
    level1.popBack();
    EXPECT_EQ(level1.getOrders().size(), 0);
    EXPECT_EQ(level1.getVolume(), 0);

    level1.addOrder(order1);
    // Test pop front.
    level1.popFront();
    EXPECT_EQ(level1.getOrders().size(), 0);
    EXPECT_EQ(level1.getVolume(), 0);
}

TEST(LevelTest, testDelete) {
    uint32_t symbol = 1;
    uint32_t quantity1 = 100;
    uint32_t quantity2 = 200;
    uint32_t quantity3 = 300;
    uint64_t price = 100;
    uint64_t id1 = 1, id2 = 2, id3 = 3;
    Order order1 = Order::newOrder(OrderType::LIMIT,
        OrderSide::Bid, id1, symbol, quantity1, price);
    Order order2 = Order::newOrder(OrderType::SBP,
        OrderSide::Bid, id2, symbol, quantity2, price);
    Order order3 = Order::newOrder(OrderType::LIMIT,
        OrderSide::Bid, id3, symbol, quantity3, price);

    Level level1 = Level(price, LevelSide::Bid, symbol);
    level1.addOrder(order1);
    level1.addOrder(order2);
    level1.addOrder(order3);

    level1.deleteOrder(order2);
    EXPECT_EQ(level1.getOrders().size(), 2);
    EXPECT_EQ(level1.getVolume(), 400);
    EXPECT_EQ(level1.getOrders().front().getOrderID(), id1);
    EXPECT_EQ(level1.getOrders().back().getOrderID(), id3);
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
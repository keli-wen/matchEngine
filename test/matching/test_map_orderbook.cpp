#include <gtest/gtest.h>
#define private public
#include "map_orderbook.h"
#undef private

using namespace UBIEngine;

TEST(MapOrderBookTest, initMapOrderBook) {
    uint32_t symbol1 = 1;
    MapOrderBook mapOrderBook1 = MapOrderBook(symbol1);
    EXPECT_EQ(mapOrderBook1.getSymbolID(), symbol1);
    EXPECT_EQ(mapOrderBook1.empty(), true);
    EXPECT_EQ(mapOrderBook1.getBidLevels().empty(), true);
    EXPECT_EQ(mapOrderBook1.getAskLevels().empty(), true);
    EXPECT_EQ(mapOrderBook1.lastTradedPrice(), 0);
}

TEST(MapOrderBookTest, addLIMITOrder) {
    uint32_t symbol = 1;
    MapOrderBook mapOrderBook = MapOrderBook(symbol);

    // init LIMIT order1.
    uint64_t order_id1 = 1;
    uint64_t quantity1 = 100;
    uint64_t price1 = 100;
    auto limitOrder1 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Bid, order_id1,
        symbol, quantity1, price1);
    
    // init LIMIT order2.
    uint64_t order_id2 = 2;
    uint64_t quantity2 = 200;
    uint64_t price2 = 101;
    auto limitOrder2 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Bid, order_id2,
        symbol, quantity2, price2);

    mapOrderBook.addOrder(limitOrder1);
    mapOrderBook.addOrder(limitOrder2);
    EXPECT_EQ(mapOrderBook.getSymbolID(), symbol);
    EXPECT_FALSE(mapOrderBook.empty());
    EXPECT_EQ(mapOrderBook.getBidLevels().size(), 2);
    EXPECT_TRUE(mapOrderBook.getAskLevels().empty());
    EXPECT_EQ(mapOrderBook.lastTradedPrice(), 0);

    // Check if the orders are in the map.
    EXPECT_TRUE(mapOrderBook.hasOrder(order_id1));
    EXPECT_TRUE(mapOrderBook.hasOrder(order_id2));
    EXPECT_FALSE(mapOrderBook.hasOrder(100));

    // Check the metadata for order2.
    auto order2 = mapOrderBook.getOrder(order_id2);
    EXPECT_EQ(order2.getOrderID(), order_id2);
    EXPECT_EQ(order2.getQuantity(), quantity2);
    EXPECT_EQ(order2.getPrice(), price2);
    EXPECT_EQ(order2.getSymbolID(), symbol);
    EXPECT_EQ(order2.getType(), OrderType::LIMIT);
    EXPECT_EQ(order2.getSide(), OrderSide::Bid);
}

TEST(MapOrderBookTest, addMarketOrder_CPBP) {
    // 目前来看不太好写测试，因为 match 会被调用。
    // 所以我们无法知道中间 order 的状态。我只能写一个简单的。

    uint32_t symbol = 1;
    MapOrderBook mapOrderBook = MapOrderBook(symbol);

    // init LIMIT order1.
    uint64_t order_id1 = 1;
    uint64_t quantity1 = 100;
    uint64_t price1 = 100;
    auto limitOrder1 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id1,
        symbol, quantity1, price1);
    
    // init SBP order2.
    uint64_t order_id2 = 2;
    uint64_t quantity2 = 200;
    uint64_t price2 = 0;
    auto cpbpOrder2 = Order::newOrder(
        OrderType::CPBP, OrderSide::Bid, order_id2,
        symbol, quantity2, price2);

    mapOrderBook.addOrder(limitOrder1);
    mapOrderBook.addOrder(cpbpOrder2);

    auto cpbpOrder_ = mapOrderBook.getOrder(order_id2);
    EXPECT_TRUE(mapOrderBook.getAskLevels().empty());
    EXPECT_EQ(cpbpOrder_.getPrice(), price1);
    EXPECT_EQ(cpbpOrder_.getType(), OrderType::LIMIT);
    // 检查 execute 后的容量
    EXPECT_FALSE(cpbpOrder_.isFilled());
    EXPECT_EQ(cpbpOrder_.getOpenQuantity(), quantity2 - quantity1);
    EXPECT_EQ(cpbpOrder_.getExecutedQuantity(), quantity1);

    // 检查 execute 的价格
    EXPECT_EQ(cpbpOrder_.getLastExecutedPrice(), price1);
    EXPECT_EQ(cpbpOrder_.getLastExecutedQuantity(), quantity1);
}

TEST(MapOrderBookTest, addMarketOrder_SBP) {
    // 目前来看不太好写测试，因为 match 会被调用。
    // 所以我们无法知道中间 order 的状态。我只能写一个简单的。

    uint32_t symbol = 1;
    MapOrderBook mapOrderBook = MapOrderBook(symbol);

    // init LIMIT order1.
    uint64_t order_id1 = 1;
    uint64_t quantity1 = 100;
    uint64_t price1 = 100;
    auto limitOrder1 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Bid, order_id1,
        symbol, quantity1, price1);
    
    // init SBP order2.
    uint64_t order_id2 = 2;
    uint64_t quantity2 = 200;
    uint64_t price2 = 0;
    auto sbpOrder2 = Order::newOrder(
        OrderType::SBP, OrderSide::Bid, order_id2,
        symbol, quantity2, price2);

    mapOrderBook.addOrder(limitOrder1);
    mapOrderBook.addOrder(sbpOrder2);

    auto sbpOrder_ = mapOrderBook.getOrder(order_id2);
    EXPECT_EQ(sbpOrder_.getPrice(), price1);
}

TEST(MapOrderBookTest, delOrder) {
    uint32_t symbol = 1;
    MapOrderBook mapOrderBook = MapOrderBook(symbol);

    // init LIMIT order1.
    uint64_t order_id1 = 1;
    uint64_t quantity1 = 100;
    uint64_t price1 = 100;
    auto limitOrder1 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Bid, order_id1,
        symbol, quantity1, price1);
    
    // init LIMIT order2.
    uint64_t order_id2 = 2;
    uint64_t quantity2 = 200;
    uint64_t price2 = 101;
    auto limitOrder2 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Bid, order_id2,
        symbol, quantity2, price2);

    mapOrderBook.addOrder(limitOrder1);
    mapOrderBook.addOrder(limitOrder2);
    // Check if the orders are in the map.
    EXPECT_TRUE(mapOrderBook.hasOrder(order_id1));
    EXPECT_TRUE(mapOrderBook.hasOrder(order_id2));

    // Delete order1.
    mapOrderBook.deleteOrder(order_id1);
    EXPECT_FALSE(mapOrderBook.hasOrder(order_id1));
    EXPECT_EQ(mapOrderBook.getBidLevels().size(), 1);

    // Delete order2.
    mapOrderBook.deleteOrder(order_id2);
    EXPECT_FALSE(mapOrderBook.hasOrder(order_id2));
    EXPECT_TRUE(mapOrderBook.empty());

    // Delete a non-existing order.
    // Expect a runtime error.
    EXPECT_THROW(mapOrderBook.deleteOrder(100), std::runtime_error);
}

TEST(MapOrderBookTest, canMatchOrder) {
    uint32_t symbol = 1;
    MapOrderBook mapOrderBook = MapOrderBook(symbol);

    // init LIMIT order1.
    uint64_t order_id1 = 1;
    uint64_t quantity1 = 200;
    uint64_t ask_price = 100;
    auto limitOrder1 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id1,
        symbol, quantity1, ask_price);
    EXPECT_EQ(limitOrder1.getPrice(), ask_price);
    EXPECT_EQ(limitOrder1.getQuantity(), quantity1);
    
    // init LIMIT order2.
    uint64_t order_id2 = 2;
    uint64_t quantity2 = 100;
    uint64_t bid_price = 200;
    auto limitOrder2 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Bid, order_id2,
        symbol, quantity2, bid_price);
    EXPECT_EQ(limitOrder2.getPrice(), bid_price);
    EXPECT_EQ(limitOrder2.getQuantity(), quantity2);
    EXPECT_EQ(limitOrder2.getOpenQuantity(), quantity2);
    EXPECT_EQ(limitOrder2.getExecutedQuantity(), 0);

    // init LIMIT order3.
    uint64_t order_id3 = 3;
    uint64_t quantity3 = 300;
    auto limitOrder3 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Bid, order_id3,
        symbol, quantity3, bid_price);
    EXPECT_EQ(limitOrder3.getPrice(), bid_price);
    EXPECT_EQ(limitOrder3.getQuantity(), quantity3);
    EXPECT_EQ(limitOrder3.getOpenQuantity(), quantity3);
    EXPECT_EQ(limitOrder3.getExecutedQuantity(), 0);

    mapOrderBook.addOrder(limitOrder1);
    EXPECT_EQ(mapOrderBook.getAskLevels().size(), 1);
    EXPECT_EQ(mapOrderBook.getAskLevels().begin()->second.getVolume(),
        quantity1);

    EXPECT_TRUE(mapOrderBook.canMatchOrder(limitOrder2));
    EXPECT_FALSE(mapOrderBook.canMatchOrder(limitOrder3));
}

TEST(MapOrderBookTest, top5Match_equal) {
    uint32_t symbol = 1;
    MapOrderBook mapOrderBook = MapOrderBook(symbol);

    // init LIMIT order1.
    uint64_t order_id1 = 1;
    uint64_t quantity1 = 100;
    uint64_t price1 = 100;
    auto limitOrder1 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id1,
        symbol, quantity1, price1);
    EXPECT_EQ(limitOrder1.getPrice(), price1);
    EXPECT_EQ(limitOrder1.getQuantity(), quantity1);
    
    // init LIMIT order2.
    uint64_t order_id2 = 2;
    uint64_t quantity2 = 200;
    uint64_t price2 = 101;
    auto limitOrder2 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id2,
        symbol, quantity2, price2);
    EXPECT_EQ(limitOrder2.getPrice(), price2);
    EXPECT_EQ(limitOrder2.getQuantity(), quantity2);
    EXPECT_EQ(limitOrder2.getOpenQuantity(), quantity2);
    EXPECT_EQ(limitOrder2.getExecutedQuantity(), 0);

    // init LIMIT order3.
    uint64_t order_id3 = 3;
    uint64_t quantity3 = 300;
    uint64_t price3 = 102;
    auto limitOrder3 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id3,
        symbol, quantity3, price3);
    EXPECT_EQ(limitOrder3.getPrice(), price3);
    EXPECT_EQ(limitOrder3.getQuantity(), quantity3);
    EXPECT_EQ(limitOrder3.getOpenQuantity(), quantity3);
    EXPECT_EQ(limitOrder3.getExecutedQuantity(), 0);

    // init TOP5_IOC_CANCEL order.
    uint64_t order_id4 = 4;
    uint64_t quantity4 = 600;
    uint64_t price4 = 0;
    auto top5Order = Order::newOrder(
        OrderType::TOP5_IOC_CANCEL, OrderSide::Bid, order_id4,
        symbol, quantity4, price4);
    EXPECT_EQ(top5Order.getPrice(), price4);
    EXPECT_EQ(top5Order.getQuantity(), quantity4);
    EXPECT_EQ(top5Order.getOpenQuantity(), quantity4);
    EXPECT_EQ(top5Order.getExecutedQuantity(), 0);

    mapOrderBook.addOrder(limitOrder1);
    mapOrderBook.addOrder(limitOrder2);
    mapOrderBook.addOrder(limitOrder3);
    EXPECT_EQ(mapOrderBook.getAskLevels().size(), 3);

    mapOrderBook.addOrder(top5Order);
    EXPECT_TRUE(mapOrderBook.getAskLevels().empty());
    EXPECT_TRUE(mapOrderBook.getBidLevels().empty());
    EXPECT_TRUE(mapOrderBook.empty());
}

TEST(MapOrderBookTest, top5Match_notFill) {
    uint32_t symbol = 1;
    MapOrderBook mapOrderBook = MapOrderBook(symbol);

    // init LIMIT order1.
    uint64_t order_id1 = 1;
    uint64_t quantity1 = 100;
    uint64_t price1 = 100;
    auto limitOrder1 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id1,
        symbol, quantity1, price1);
    EXPECT_EQ(limitOrder1.getPrice(), price1);
    EXPECT_EQ(limitOrder1.getQuantity(), quantity1);
    
    // init LIMIT order2.
    uint64_t order_id2 = 2;
    uint64_t quantity2 = 200;
    uint64_t price2 = 101;
    auto limitOrder2 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id2,
        symbol, quantity2, price2);
    EXPECT_EQ(limitOrder2.getPrice(), price2);
    EXPECT_EQ(limitOrder2.getQuantity(), quantity2);
    EXPECT_EQ(limitOrder2.getOpenQuantity(), quantity2);
    EXPECT_EQ(limitOrder2.getExecutedQuantity(), 0);

    // init LIMIT order3.
    uint64_t order_id3 = 3;
    uint64_t quantity3 = 300;
    uint64_t price3 = 102;
    auto limitOrder3 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id3,
        symbol, quantity3, price3);
    EXPECT_EQ(limitOrder3.getPrice(), price3);
    EXPECT_EQ(limitOrder3.getQuantity(), quantity3);
    EXPECT_EQ(limitOrder3.getOpenQuantity(), quantity3);
    EXPECT_EQ(limitOrder3.getExecutedQuantity(), 0);

    // init TOP5_IOC_CANCEL order.
    uint64_t order_id4 = 4;
    uint64_t quantity4 = 700;
    uint64_t price4 = 0;
    auto top5Order = Order::newOrder(
        OrderType::TOP5_IOC_CANCEL, OrderSide::Bid, order_id4,
        symbol, quantity4, price4);
    EXPECT_EQ(top5Order.getPrice(), price4);
    EXPECT_EQ(top5Order.getQuantity(), quantity4);
    EXPECT_EQ(top5Order.getOpenQuantity(), quantity4);
    EXPECT_EQ(top5Order.getExecutedQuantity(), 0);

    mapOrderBook.addOrder(limitOrder1);
    mapOrderBook.addOrder(limitOrder2);
    mapOrderBook.addOrder(limitOrder3);
    EXPECT_EQ(mapOrderBook.getAskLevels().size(), 3);

    mapOrderBook.addOrder(top5Order);
    EXPECT_TRUE(mapOrderBook.getAskLevels().empty());
    EXPECT_TRUE(mapOrderBook.getBidLevels().empty());
    EXPECT_TRUE(mapOrderBook.empty());
}

TEST(MapOrderBookTest, top5Match_exceed) {
    uint32_t symbol = 1;
    MapOrderBook mapOrderBook = MapOrderBook(symbol);

    // init LIMIT order1.
    uint64_t order_id1 = 1;
    uint64_t quantity1 = 100;
    uint64_t price1 = 100;
    auto limitOrder1 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id1,
        symbol, quantity1, price1);
    EXPECT_EQ(limitOrder1.getPrice(), price1);
    EXPECT_EQ(limitOrder1.getQuantity(), quantity1);
    
    // init LIMIT order2.
    uint64_t order_id2 = 2;
    uint64_t quantity2 = 200;
    uint64_t price2 = 101;
    auto limitOrder2 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id2,
        symbol, quantity2, price2);
    EXPECT_EQ(limitOrder2.getPrice(), price2);
    EXPECT_EQ(limitOrder2.getQuantity(), quantity2);
    EXPECT_EQ(limitOrder2.getOpenQuantity(), quantity2);
    EXPECT_EQ(limitOrder2.getExecutedQuantity(), 0);

    // init LIMIT order3.
    uint64_t order_id3 = 3;
    uint64_t quantity3 = 300;
    uint64_t price3 = 102;
    auto limitOrder3 = Order::newOrder(
        OrderType::LIMIT, OrderSide::Ask, order_id3,
        symbol, quantity3, price3);
    EXPECT_EQ(limitOrder3.getPrice(), price3);
    EXPECT_EQ(limitOrder3.getQuantity(), quantity3);
    EXPECT_EQ(limitOrder3.getOpenQuantity(), quantity3);
    EXPECT_EQ(limitOrder3.getExecutedQuantity(), 0);

    // init TOP5_IOC_CANCEL order.
    uint64_t order_id4 = 4;
    uint64_t quantity4 = 500;
    uint64_t price4 = 0;
    auto top5Order = Order::newOrder(
        OrderType::TOP5_IOC_CANCEL, OrderSide::Bid, order_id4,
        symbol, quantity4, price4);
    EXPECT_EQ(top5Order.getPrice(), price4);
    EXPECT_EQ(top5Order.getQuantity(), quantity4);
    EXPECT_EQ(top5Order.getOpenQuantity(), quantity4);
    EXPECT_EQ(top5Order.getExecutedQuantity(), 0);

    mapOrderBook.addOrder(limitOrder1);
    mapOrderBook.addOrder(limitOrder2);
    mapOrderBook.addOrder(limitOrder3);
    EXPECT_EQ(mapOrderBook.getAskLevels().size(), 3);

    mapOrderBook.addOrder(top5Order);
    // 应该能恰好留下最后一个 limit order.
    EXPECT_EQ(mapOrderBook.getAskLevels().size(), 1);
    EXPECT_TRUE(mapOrderBook.hasOrder(order_id3));
    EXPECT_EQ(mapOrderBook.getOrder(order_id3).getOpenQuantity(), 100);
    EXPECT_EQ(mapOrderBook.getOrder(order_id3).getExecutedQuantity(), 200);
    EXPECT_EQ(mapOrderBook.getOrder(order_id3).getLastExecutedQuantity(), 200);
    EXPECT_EQ(mapOrderBook.getOrder(order_id3).getLastExecutedPrice(), price3);
    EXPECT_TRUE(mapOrderBook.getBidLevels().empty());
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
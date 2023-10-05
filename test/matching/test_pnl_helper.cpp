#include <gtest/gtest.h>
#include "pnl_helper.h"

using namespace UBIEngine;

TEST(PnlHelperTest, initPnlHelper) {
    uint64_t prev_close_price = 100;
    uint32_t prev_position = 100;
    PnlHelper pnl_helper(prev_close_price, prev_position);
    EXPECT_EQ(pnl_helper.getPreviousClosePrice(), 100);
    EXPECT_EQ(pnl_helper.getPosition(), 100);
    EXPECT_EQ(pnl_helper.getPrevPosition(), 100);
    EXPECT_EQ(pnl_helper.getCash(), 0);
}

TEST(PnlHelperTest, calculatePnl) {
    /* easy version */
    uint64_t prev_close_price = 100;
    uint32_t prev_position = 100;
    PnlHelper pnl_helper(prev_close_price, prev_position);
    EXPECT_EQ(pnl_helper.getPreviousClosePrice(), 100);
    EXPECT_EQ(pnl_helper.getPosition(), 100);
    EXPECT_EQ(pnl_helper.getPrevPosition(), 100);
    EXPECT_EQ(pnl_helper.getCash(), 0);

    int64_t pnl = pnl_helper.calculatePnl(100);
    EXPECT_EQ(pnl, 0);

    pnl = pnl_helper.calculatePnl(200);
    EXPECT_EQ(pnl, 10000);

    pnl = pnl_helper.calculatePnl(50);
    EXPECT_EQ(pnl, -5000);
}

TEST(PnlHelperTest, updateAccount) {
    uint64_t prev_close_price = 100;
    uint32_t prev_position = 100;
    PnlHelper pnl_helper(prev_close_price, prev_position);
    EXPECT_EQ(pnl_helper.getPreviousClosePrice(), 100);
    EXPECT_EQ(pnl_helper.getPosition(), 100);
    EXPECT_EQ(pnl_helper.getPrevPosition(), 100);
    EXPECT_EQ(pnl_helper.getCash(), 0);

    pnl_helper.updateAccount(OrderSide::Bid, 100, 100);
    EXPECT_EQ(pnl_helper.getPreviousClosePrice(), 100);
    EXPECT_EQ(pnl_helper.getPosition(), 200);
    EXPECT_EQ(pnl_helper.getPrevPosition(), 100);
    EXPECT_EQ(pnl_helper.getCash(), -10000);

    pnl_helper.updateAccount(OrderSide::Ask, 100, 100);
    EXPECT_EQ(pnl_helper.getPreviousClosePrice(), 100);
    EXPECT_EQ(pnl_helper.getPosition(), 100);
    EXPECT_EQ(pnl_helper.getPrevPosition(), 100);
    EXPECT_EQ(pnl_helper.getCash(), 0);
}

TEST(PnlHelperTest, toString) {
    uint64_t prev_close_price = 100;
    uint32_t prev_position = 100;
    PnlHelper pnl_helper(prev_close_price, prev_position);
    EXPECT_EQ(pnl_helper.getPreviousClosePrice(), 100);
    EXPECT_EQ(pnl_helper.getPosition(), 100);
    EXPECT_EQ(pnl_helper.getPrevPosition(), 100);
    EXPECT_EQ(pnl_helper.getCash(), 0);

    std::string pnl_helper_string = pnl_helper.toString();
    EXPECT_EQ(pnl_helper_string,
              "Previous Close Price: 100\n"
              "Previous Position: 100\n"
              "Position: 100\n"
              "Cash: 0\n");

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

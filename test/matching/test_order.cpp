#include <gtest/gtest.h>
#include "order.h"

TEST(OrderTest, SomeTestName) {
    // 这里写你的测试代码。
    EXPECT_EQ(1, 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

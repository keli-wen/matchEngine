cmake_minimum_required(VERSION 3.10)

# 添加 Google Test 的路径
set(GTEST_PATH $ENV{HOME}/googletest)

# 链接 Google Test 到项目中
include_directories(${GTEST_PATH}/googletest/include)
link_directories(${GTEST_PATH}/build/lib)


# 添加你的测试源文件
add_executable(test_order matching/test_order.cpp)

# 链接 Google Test 库
target_link_libraries(test_order gtest gtest_main pthread)

# 添加测试
add_test(NAME test_order COMMAND test_order)

# 添加专门的测试运行目标
add_custom_target(test_order_run COMMAND test_order)
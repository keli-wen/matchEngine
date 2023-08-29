cmake_minimum_required(VERSION 3.10)

project(Engine)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)

# 添加 Google Test 的路径
set(GTEST_PATH $ENV{HOME}/googletest)

# Boost 配置
set(BOOST_INCLUDE_PATH $ENV{HOME}/local/include/boost)
set(BOOST_LIB_PATH $ENV{HOME}/local/lib/boost)
find_package(Boost REQUIRED)
include_directories(${BOOST_INCLUDE_PATH})

# 链接 Boost 库
link_directories(${BOOST_LIB_PATH})

# 包含项目的 include 目录
include_directories(${CMAKE_SOURCE_DIR}/include)
# 添加项目的 include/matching 目录，为了方便引入头文件
include_directories(${CMAKE_SOURCE_DIR}/include/matching)


# 启用测试功能
enable_testing()

# 添加子目录
add_subdirectory(test)


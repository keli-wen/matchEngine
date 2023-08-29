cmake_minimum_required(VERSION 3.10)

project(Engine)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set(TRADER_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(TRADER_INSTALL_BIN_DIR ${CMAKE_INSTALL_BINDIR})

# 添加 Google Test 的路径
set(GTEST_PATH $ENV{HOME}/googletest)
include_directories(${GTEST_PATH}/googletest/include)
link_directories(${GTEST_PATH}/build/lib)

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

# ------------------------------------------------------------------------------
# UBIEngine Library
# ------------------------------------------------------------------------------
file(GLOB_RECURSE LIB_HEADER_FILES "include/*.h")
file(GLOB_RECURSE LIB_SOURCE_FILES "src/*.cpp")
add_library(trader_lib ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
install(TARGETS trader_lib DESTINATION ${TRADER_INSTALL_LIB_DIR})

# 启用测试功能
enable_testing()

# 临时的后续需要修改!!
# 添加你的测试源文件
add_executable(test_order test/matching/test_order.cpp)
add_executable(test_level test/matching/test_level.cpp)
add_executable(test_symbol test/matching/test_symbol.cpp)

# 链接 Google Test 库
target_link_libraries(test_order gtest gtest_main pthread)
target_link_libraries(test_order trader_lib)
# Test Level.cpp
target_link_libraries(test_level gtest gtest_main pthread)
target_link_libraries(test_level trader_lib)
# Test Symbol.cpp
target_link_libraries(test_symbol gtest gtest_main pthread)
target_link_libraries(test_symbol trader_lib)

# 添加测试
add_test(NAME test_order COMMAND test_order)
add_test(NAME test_level COMMAND test_level)
add_test(NAME test_symbol COMMAND test_symbol)

# 添加专门的测试运行目标
add_custom_target(test_order_run COMMAND test_order)
add_custom_target(test_level_run COMMAND test_level)
add_custom_target(test_symbol_run COMMAND test_symbol)
# 添加子目录
# add_subdirectory(test)


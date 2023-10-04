cmake_minimum_required(VERSION 3.10)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

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
set(GTEST_PATH "${CMAKE_SOURCE_DIR}/external/googletest")
# 注意：现在 Google Test 也使用 CMake。所以，最佳实践是通过 add_subdirectory 来包含它。
# 这样，你可以直接链接到 gtest 和 gtest_main 目标。
add_subdirectory(${GTEST_PATH})

# Boost 配置。
find_package(Boost REQUIRED COMPONENTS filesystem system)

# 包含项目的 include 目录
include_directories(${CMAKE_SOURCE_DIR}/include)
# 添加项目的 include/matching 目录，为了方便引入头文件
include_directories(${CMAKE_SOURCE_DIR}/include/matching)
include_directories(${CMAKE_SOURCE_DIR}/include/utils)
include_directories(${CMAKE_SOURCE_DIR}/include/market)

# ------------------------------------------------------------------------------
# UBIEngine Library
# ------------------------------------------------------------------------------
file(GLOB_RECURSE LIB_HEADER_FILES "include/*.h")
file(GLOB_RECURSE LIB_SOURCE_FILES "src/*.cpp")
list(REMOVE_ITEM LIB_SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")
add_library(trader_lib ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
install(TARGETS trader_lib DESTINATION ${TRADER_INSTALL_LIB_DIR})

add_executable(engine_main src/main.cpp)
# 必须链接 pthread 库，否则会报错。
target_link_libraries(engine_main trader_lib pthread)
# 记得链接 Boost 中的 filesystem
target_link_libraries(engine_main Boost::filesystem)

# 启用测试功能
enable_testing()

# 临时的后续需要修改!!
# 添加你的测试源文件
add_executable(test_order test/matching/test_order.cpp)
add_executable(test_level test/matching/test_level.cpp)
add_executable(test_symbol test/matching/test_symbol.cpp)
add_executable(test_maporderbook test/matching/test_map_orderbook.cpp)
add_executable(test_pnlhelper test/matching/test_pnl_helper.cpp)

# 链接 Google Test 库
target_link_libraries(test_order gtest gtest_main pthread)
target_link_libraries(test_order trader_lib)
# Test Level.cpp
target_link_libraries(test_level gtest gtest_main pthread)
target_link_libraries(test_level trader_lib)
# Test Symbol.cpp
target_link_libraries(test_symbol gtest gtest_main pthread)
target_link_libraries(test_symbol trader_lib)
# Test MapOrderbook.cpp
target_link_libraries(test_maporderbook gtest gtest_main pthread)
target_link_libraries(test_maporderbook trader_lib)
# Test PnlHelper.cpp
target_link_libraries(test_pnlhelper gtest gtest_main pthread)
target_link_libraries(test_pnlhelper trader_lib)

# 添加测试
add_test(NAME test_order COMMAND test_order)
add_test(NAME test_level COMMAND test_level)
add_test(NAME test_symbol COMMAND test_symbol)
add_test(NAME test_maporderbook COMMAND test_maporderbook)
add_test(NAME test_pnlhelper COMMAND test_pnlhelper)

# 添加专门的测试运行目标
add_custom_target(test_order_run COMMAND test_order)
add_custom_target(test_level_run COMMAND test_level)
add_custom_target(test_symbol_run COMMAND test_symbol)
add_custom_target(test_maporderbook_run COMMAND test_maporderbook)
add_custom_target(test_pnlhelper_run COMMAND test_pnlhelper)
add_custom_target(engine_main_run COMMAND engine_main)
# 添加子目录
# add_subdirectory(test)


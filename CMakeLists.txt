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
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/include/matching
  ${CMAKE_SOURCE_DIR}/include/utils
  ${CMAKE_SOURCE_DIR}/include/market
)

# ------------------------------------------------------------------------------
# UBIEngine Library
# ------------------------------------------------------------------------------
file(GLOB_RECURSE LIB_HEADER_FILES "include/*.h")
file(GLOB_RECURSE LIB_SOURCE_FILES "src/*.cpp")
list(REMOVE_ITEM LIB_SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/main.cpp")
add_library(trader_lib ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
install(TARGETS trader_lib DESTINATION ${TRADER_INSTALL_LIB_DIR})

# Main executable configuration.
add_executable(engine_main src/main.cpp)
target_link_libraries(engine_main trader_lib pthread Boost::filesystem)

# 启用测试功能
enable_testing()

# Define test names and their respective source files
set(TEST_NAMES order level symbol maporderbook pnlhelper)
set(TEST_SOURCE_FILES
    test/matching/test_order.cpp
    test/matching/test_level.cpp
    test/matching/test_symbol.cpp
    test/matching/test_map_orderbook.cpp
    test/matching/test_pnl_helper.cpp
)

# Get the length of the lists.
list(LENGTH TEST_NAMES NUM_TESTS)

# Subtract 1 as list indices start from 0.
math(EXPR NUM_TESTS "${NUM_TESTS} - 1")

# Iterate over the indices of the lists to create executable, link libraries,
# and add tests and targets.
# Note!: If CMAKE_VERSION >= 3.22.0, ZIP_LISTS is a better choice.
foreach(INDEX RANGE ${NUM_TESTS})
  # Get the test name and source file from the lists using the current index.
  list(GET TEST_NAMES ${INDEX} TEST_NAME)
  list(GET TEST_SOURCE_FILES ${INDEX} TEST_SRC)

  # Create the executable, link libraries, and add tests and targets.
  add_executable(test_${TEST_NAME} ${TEST_SRC})
  target_link_libraries(test_${TEST_NAME} gtest gtest_main pthread trader_lib)
  add_test(NAME test_${TEST_NAME} COMMAND test_${TEST_NAME})
  add_custom_target(test_${TEST_NAME}_run COMMAND test_${TEST_NAME})
endforeach()

# Special target for engine_main.
add_custom_target(engine_main_run COMMAND engine_main)

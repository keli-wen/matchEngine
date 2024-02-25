cmake_minimum_required(VERSION 3.10)
project(Benchmarks)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Boost 配置。
find_package(Boost REQUIRED COMPONENTS filesystem)

# 包含头文件目录。
message(STATUS "Include dir: ${CMAKE_SOURCE_DIR}/../include")
include_directories(${CMAKE_SOURCE_DIR}/../include)

# 添加子目录
add_subdirectory(IO)

cmake_minimum_required(VERSION 3.10)
project(IOBenchmarkTest)

# 创建可执行文件 io_benchmark_test
add_executable(io_benchmark_test test_reader.cpp)
target_link_libraries(io_benchmark_test Boost::filesystem pthread)

# 链接主项目的源文件（如果需要）
# target_link_libraries(io_benchmark_test src_lib)

# 添加一个自定义目标来运行测试
add_custom_target(
    run_io_benchmark_test
    COMMAND io_benchmark_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
#include <algorithm>
#include <boost/filesystem.hpp>
#include <chrono>
#include <iostream>
#include <random>

#include "io/reader.h"
using namespace UBIEngine;
#define RANDOM_TRAVERSE 1
// The number of work threads. (5 is equal to the Hackathon's requirement.)
constexpr int work_threads_cnt = 5;

// 定义一个 Timer 用来传入一个函数指针和对应的参数，然后可以设定运行次数，最后给出平均运行时间。
class Timer {
   public:
    template <typename F, typename... Args>
    static double timeit(F&& func, Args&&... args) {
        auto start = std::chrono::high_resolution_clock::now();
        std::forward<F>(func)(std::forward<Args>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        // Use milliseconds as the unit of time.
        std::chrono::duration<double, std::milli> diff = end - start;
        return diff.count();
    }

    template <typename F, typename... Args>
    static double timeit(F&& func, int repeat, Args&&... args) {
        double total_time = 0;
        for (int i = 0; i < repeat; i++) {
            total_time += timeit(std::forward<F>(func), std::forward<Args>(args)...);
        }
        return total_time / repeat;
    }
};

template <typename T>
void traverse(const std::vector<T>& arr) {
    // Traverse the array elements.
    for (auto& element : arr) {
        auto& e = element;
    }
}

template <typename T>
void traverse(const T* arr, size_t size) {
    // Traverse the array elements.
    for (size_t i = 0; i < size; i++) {
        auto& e = arr[i];
    }
}

std::vector<size_t> generate_random_indices(size_t size) {
    std::vector<size_t> indices(size);
    std::iota(indices.begin(), indices.end(), 0);  // 填充从 0 开始的连续整数
    std::shuffle(indices.begin(), indices.end(),
                 std::default_random_engine(static_cast<unsigned int>(std::time(nullptr))));
    return indices;
}

template <typename T>
void random_traverse(const std::vector<T>& arr, const std::vector<size_t>& indices) {
    for (auto index : indices) {
        auto& e = arr[index];
    }
}

template <typename T>
void random_traverse(const T* arr, size_t size, const std::vector<size_t>& indices) {
    for (auto index : indices) {
        auto& e = arr[index];
    }
}

template <typename T>
void traverse_thread_task(T data, int& generate_time, size_t size = 0) {
    std::thread threads[work_threads_cnt];

    auto start_time = std::chrono::high_resolution_clock::now();
#ifdef RANDOM_TRAVERSE
    size_t data_size;
    if constexpr (std::is_pointer_v<T>) {
        data_size = size;
    } else {
        data_size = data.size();
    }
    auto indices = generate_random_indices(data_size);
    auto mid_time = std::chrono::high_resolution_clock::now();
    std::cout << "Cost time of generating random indices: "
              << std::chrono::duration<double, std::milli>(mid_time - start_time).count() << "ms"
              << std::endl;
    generate_time = std::chrono::duration<double, std::milli>(mid_time - start_time).count();
#else
    auto mid_time = std::chrono::high_resolution_clock::now();
    generate_time = 0;
#endif

    for (int i = 0; i < work_threads_cnt; ++i) {
        threads[i] = std::thread([=]() {
            if constexpr (std::is_pointer_v<T>) {
#ifdef RANDOM_TRAVERSE
                random_traverse(data, size, indices);
#else
                traverse(data, size);
#endif
            } else {
#ifdef RANDOM_TRAVERSE
                random_traverse(data, indices);
#else
                traverse(data);
#endif
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Cost time of traversing: "
              << std::chrono::duration<double, std::milli>(end_time - mid_time).count() << "ms"
              << std::endl;
}

namespace BenchmarkTask {
template <typename T>
void read_sync(const std::string& file_path, int& generate_time) {
    auto start_read_time = std::chrono::high_resolution_clock::now();
    auto order_logs = IO::reader_sync<T>(file_path);
    auto end_read_time = std::chrono::high_resolution_clock::now();
    std::cout << "Cost time of reading: "
              << std::chrono::duration<double, std::milli>(end_read_time - start_read_time).count()
              << "ms" << std::endl;

    traverse_thread_task(order_logs, generate_time);
}

template <typename T>
void read_concurrent(const std::string& file_path, int& generate_time, size_t thread_cnt) {
    if constexpr (std::is_same_v<T, IO::alpha>) {
        auto start_read_time = std::chrono::high_resolution_clock::now();
        auto alphas = IO::read_alpha(file_path, thread_cnt);
        auto end_read_time = std::chrono::high_resolution_clock::now();
        std::cout
            << "Cost time of reading: "
            << std::chrono::duration<double, std::milli>(end_read_time - start_read_time).count()
            << "ms" << std::endl;

        traverse_thread_task(alphas, generate_time);
    } else {
        auto start_read_time = std::chrono::high_resolution_clock::now();
        auto order_logs = IO::read_order_log(file_path, thread_cnt);
        auto end_read_time = std::chrono::high_resolution_clock::now();
        std::cout
            << "Cost time of reading: "
            << std::chrono::duration<double, std::milli>(end_read_time - start_read_time).count()
            << "ms" << std::endl;

        traverse_thread_task(order_logs, generate_time);
    }
}

template <typename T>
void read_mmap(const std::string& file_path, int& generate_time) {
    // Use perfect forwarding to avoid unnecessary copy.
    auto start_read_time = std::chrono::high_resolution_clock::now();
    auto&& [order_logs, length] = IO::reader_mmap<T>(file_path);
    auto end_read_time = std::chrono::high_resolution_clock::now();
    std::cout << "Cost time of reading: "
              << std::chrono::duration<double, std::milli>(end_read_time - start_read_time).count()
              << "ms" << std::endl;

    traverse_thread_task(order_logs.get(), generate_time, length);
}
}  // namespace BenchmarkTask

int main() {
#ifdef RANDOM_TRAVERSE
    std::cout << "Random traverse benchmark" << std::endl;
#else
    std::cout << "Sequential traverse benchmark" << std::endl;
#endif
    // Assume the input dataset path is {Project_Dir}/data/input_data.
    boost::filesystem::path p("../../data/input_data/");
    for (auto& entry : boost::filesystem::directory_iterator(p)) {
        std::string dataset_dir_path = entry.path().string();
        std::string fileActPath = entry.path().filename().string();
        boost::filesystem::path dataset_abs_path = boost::filesystem::absolute(dataset_dir_path);
        std::cout << "Dataset path: " << dataset_abs_path << " fileActPath: " << fileActPath
                  << std::endl;

        // ! 不能用 Timer.timeit。因为 :: 用来访问 static member 和 namespace 中的成员。
        // ! 而 . 用来访问对象的成员。

        // ! Assign the benchmark repeat times.
        const static int repeat = 2;
        int gentime_order_sync = 0, gentime_order_concurrent = 0, gentime_order_mmap = 0,
            gentime_alpha_sync = 0, gentime_alpha_concurrent = 0, gentime_alpha_mmap = 0;
        auto cost_read_order_sync =
            Timer::timeit(BenchmarkTask::read_sync<IO::order_log>, /*repeat=*/repeat,
                          dataset_dir_path + "/order_log", gentime_order_sync);
        auto cost_read_order_concurrent =
            Timer::timeit(BenchmarkTask::read_concurrent<IO::order_log>, /*repeat=*/repeat,
                          dataset_dir_path + "/order_log", gentime_order_concurrent,
                          std::thread::hardware_concurrency());
        auto cost_read_order_mmap =
            Timer::timeit(BenchmarkTask::read_mmap<IO::order_log>, /*repeat=*/repeat,
                          dataset_dir_path + "/order_log", gentime_order_mmap);

        auto cost_read_alpha_sync =
            Timer::timeit(BenchmarkTask::read_sync<IO::alpha>,
                          /*repeat=*/repeat, dataset_dir_path + "/alpha", gentime_alpha_sync);
        auto cost_read_alpha_concurrent =
            Timer::timeit(BenchmarkTask::read_concurrent<IO::alpha>, /*repeat=*/repeat,
                          dataset_dir_path + "/alpha", gentime_alpha_concurrent,
                          std::thread::hardware_concurrency());
        auto cost_read_alpha_mmap =
            Timer::timeit(BenchmarkTask::read_mmap<IO::alpha>,
                          /*repeat=*/repeat, dataset_dir_path + "/alpha", gentime_alpha_mmap);

        std::cout << "Cost time of reading order_log: (repeat " << repeat << ") "
                  << (cost_read_order_sync - gentime_order_sync) / 1000 << "s" << std::endl;
        std::cout << "Cost time of reading order_log with concurrent: (repeat " << repeat
                  << " thread cnt " << std::thread::hardware_concurrency() << ") "
                  << (cost_read_order_concurrent - gentime_order_concurrent) / 1000 << "s"
                  << std::endl;
        std::cout << "Cost time of reading order_log with mmap: (repeat " << repeat << ") "
                  << (cost_read_order_mmap - gentime_order_mmap) / 1000 << "s" << std::endl;
        std::cout << "Cost time of reading alpha: (repeat " << repeat << ") "
                  << (cost_read_alpha_sync - gentime_alpha_sync) / 1000 << "s" << std::endl;
        std::cout << "Cost time of reading alpha with concurrent: (repeat " << repeat
                  << " thread cnt " << std::thread::hardware_concurrency() << ") "
                  << (cost_read_alpha_concurrent - gentime_alpha_concurrent) / 1000 << "s"
                  << std::endl;
        std::cout << "Cost time of reading alpha with mmap: (repeat " << repeat << ") "
                  << (cost_read_alpha_mmap - gentime_alpha_mmap) / 1000 << "s" << std::endl;
    }
    return 0;
}
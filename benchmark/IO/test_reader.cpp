#include <boost/filesystem.hpp>
#include <chrono>
#include <iostream>

#include "io/reader.h"
using namespace UBIEngine;

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

int main() {
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
        const static int repeat = 2;
        auto cost_read_order_sync = Timer::timeit(IO::reader_sync<IO::order_log>, /*repeat=*/repeat,
                                                  dataset_dir_path + "/order_log");
        auto cost_read_order_concurrent =
            Timer::timeit(IO::read_order_log, /*repeat=*/repeat, dataset_dir_path + "/order_log",
                          std::thread::hardware_concurrency());
        auto cost_read_order_mmap = Timer::timeit(IO::reader_mmap<IO::order_log>, /*repeat=*/repeat,
                                                  dataset_dir_path + "/order_log");

        auto cost_read_alpha_sync = Timer::timeit(IO::reader_sync<IO::alpha>, /*repeat=*/repeat,
                                                  dataset_dir_path + "/alpha");
        auto cost_read_alpha_concurrent =
            Timer::timeit(IO::read_alpha, /*repeat=*/repeat, dataset_dir_path + "/alpha",
                          std::thread::hardware_concurrency());
        auto cost_read_alpha_mmap = Timer::timeit(IO::reader_mmap<IO::alpha>, /*repeat=*/repeat,
                                                  dataset_dir_path + "/alpha");

        // Check if the two methods read the same data.
        auto data_sync = IO::reader_sync<IO::order_log>(dataset_dir_path + "/order_log");
        auto data_mmap = IO::reader_mmap<IO::order_log>(dataset_dir_path + "/order_log");
        // if (data_sync.size() != data_mmap.size()) {
        //     std::cerr << "The two methods read different data!" << std::endl;
        //     std::cerr << "data_sync.size() = " << data_sync.size()
        //               << " data_mmap.size() = " << data_mmap.size() << std::endl;
        //     return 1;
        // }

        std::cout << "Cost time of reading order_log: (repeat " << repeat << ") "
                  << cost_read_order_sync / 1000 << "s" << std::endl;
        std::cout << "Cost time of reading order_log with concurrent: (repeat " << repeat
                  << " thread cnt " << std::thread::hardware_concurrency() << ") "
                  << cost_read_order_concurrent / 1000 << "s" << std::endl;
        std::cout << "Cost time of reading order_log with mmap: (repeat " << repeat << ") "
                  << cost_read_order_mmap / 1000 << "s" << std::endl;
        std::cout << "Cost time of reading alpha: (repeat " << repeat << ") "
                  << cost_read_alpha_sync / 1000 << "s" << std::endl;
        std::cout << "Cost time of reading alpha with concurrent: (repeat " << repeat
                  << " thread cnt " << std::thread::hardware_concurrency() << ") "
                  << cost_read_alpha_concurrent / 1000 << "s" << std::endl;
        std::cout << "Cost time of reading alpha with mmap: (repeat " << repeat << ") "
                  << cost_read_alpha_mmap / 1000 << "s" << std::endl;
    }
    return 0;
}
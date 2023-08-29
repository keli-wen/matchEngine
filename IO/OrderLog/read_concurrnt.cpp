#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <chrono>

struct order_log {
    char instrument_id[8];
    long timestamp;
    int type;
    int direction;
    int volume;
    double price_off;
} __attribute__((packed));

void readChunk(const std::string& filename, long start, long end, std::vector<order_log>& data) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file inside thread!" << std::endl;
        return;
    }

    file.seekg(start * sizeof(order_log), std::ios::beg);
    
    for (long i = start; i < end; ++i) {
        order_log entry;
        file.read((char*)&entry, sizeof(order_log));
        data.push_back(entry);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_file>" << std::endl;
        return 1;
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    std::ifstream file(argv[1], std::ios::binary);  
    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    file.seekg(0, std::ios::end);
    long total_entries = file.tellg() / sizeof(order_log);
    file.seekg(0, std::ios::beg);

    // int thread_count = std::thread::hardware_concurrency();
    int thread_count = 8;
    long chunk_size = total_entries / thread_count;
    long remainder = total_entries % thread_count;
    std::cout << "Current thread count: " << thread_count << std::endl;

    std::vector<std::vector<order_log>> thread_data(thread_count);
    std::vector<std::thread> threads;

    long start = 0;
    for (int i = 0; i < thread_count; ++i) {
        long extra = (i < remainder) ? 1 : 0;  // Distribute the remainder among the first few threads
        threads.emplace_back(readChunk, argv[1],
            start, start + chunk_size + extra, std::ref(thread_data[i]));
        start += chunk_size + extra;
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // 如果我们只多线程读，不考虑合并，那么速度提升是 400% 左右。
    auto before_combine_time = std::chrono::high_resolution_clock::now();

    // Concurrently combine data from all threads.
    // Seepd up: 30%
    std::vector<order_log> combined_data(total_entries);

    // To find out where to insert data from each thread, we'll calculate offsets.
    std::vector<std::size_t> offsets(thread_data.size(), 0);
    for (std::size_t i = 1; i < thread_data.size(); ++i) {
        offsets[i] = offsets[i-1] + thread_data[i-1].size();
    }

    std::vector<std::thread> join_threads;

    for (std::size_t i = 0; i < thread_data.size(); ++i) {
        join_threads.push_back(std::thread([&, i](){
            std::copy(thread_data[i].begin(), thread_data[i].end(), combined_data.begin() + offsets[i]);
        }));
    }

    for (auto& t : join_threads) {
        t.join();
    }


    // C++17
    // std::size_t total_entries = 0;
    // for (const auto& data : thread_data) {
    //     total_entries += data.size();
    // }

    // std::vector<order_log> combined_data(total_entries);

    // // We can use an atomic to keep track of where to insert next.
    // std::atomic<std::size_t> insert_pos(0);

    // std::for_each(std::execution::par, thread_data.begin(), thread_data.end(),
    //     [&combined_data, &insert_pos](const auto& data) {
    //         // Get the current insert position and advance it by the data size.
    //         std::size_t pos = insert_pos.fetch_add(data.size());

    //         // Copy data in parallel
    //         std::copy(data.begin(), data.end(), combined_data.begin() + pos);
    //     }
    // );

    // Combine data from all threads
    // std::vector<order_log> combined_data;
    // for (const auto& data : thread_data) {
    //     combined_data.insert(combined_data.end(), data.begin(), data.end());
    // }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Read [Concurrnt] Time taken: (before combine)"
        << std::chrono::duration_cast<std::chrono::milliseconds>
        (end_time - before_combine_time).count() << "ms" << std::endl;
    std::cout << "Read [Concurrnt] Time taken: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
        << "ms" << std::endl;
    std::cout << "Please press enter to continue..." << std::endl;

    return 0;
}

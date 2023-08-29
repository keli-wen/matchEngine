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


void print_order_log(const std::vector<order_log>& data, int count = 100) {
    int cur_count = 0;
    for (const auto& entry : data) {
        std::cout << "Instrument ID: " << entry.instrument_id
                  << ", Timestamp: " << entry.timestamp
                  << ", Type: " << entry.type
                  << ", Direction: " << entry.direction
                  << ", Volume: " << entry.volume
                  << ", Price Offset: " << entry.price_off
                  << std::endl;
        ++ cur_count;
        if (cur_count >= count) break;
    }
}


/* Speed up: 3x+ */
void readChunk(const std::string& filename, long start, long end, std::vector<order_log>& data) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file inside thread!" << std::endl;
        return;
    }

    file.seekg(start * sizeof(order_log), std::ios::beg);

    // 直接在给定的位置填充数据
    for (long i = start; i < end; ++i) {
        order_log entry;
        file.read((char*)&entry, sizeof(order_log));
        data[i] = entry;
    }
}


int main(int argc, char* argv[]) {
    if (argc <= 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_file>"
                  << " <thread_count>" << std::endl;
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

    int thread_count = std::thread::hardware_concurrency();
    if (argc >= 3) thread_count = std::stoi(argv[2]);

    long chunk_size = total_entries / thread_count;
    long remainder = total_entries % thread_count;
    std::cout << "Current thread count: " << thread_count << std::endl;

    std::vector<order_log> combined_data(total_entries);
    std::vector<std::thread> threads;

    long start = 0;
    for (int i = 0; i < thread_count; ++i) {
        long extra = (i < remainder) ? 1 : 0;  // Distribute the remainder among the first few threads.
        threads.emplace_back(readChunk, argv[1],
            start, start + chunk_size + extra, std::ref(combined_data)); // 直接将预先分配的大vector传给线程。
        start += chunk_size + extra;
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Read [Concurrnt] Time taken: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
        << "ms" << std::endl;
    print_order_log(combined_data);

    return 0;
}

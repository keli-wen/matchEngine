#ifndef UBI_TRADER_IO_READER_H
#define UBI_TRADER_IO_READER_H
#include <vector>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <queue>
#include <io/type.h>

namespace UBIEngine::IO {

template <typename T>
void readChunk(
    const std::string& filename,
    long start,
    long end,
    std::vector<T>& data)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file inside thread!" << std::endl;
        return;
    }

    file.seekg(start * sizeof(T), std::ios::beg);
    for (long i = start; i < end; ++i) {
        T entry;
        file.read((char*)&entry, sizeof(T));
        data[i] = entry;
    }
}

template<typename T>
std::vector<T> reader_sync(const std::string& filePath) {
    std::vector<T> data;
    std::ifstream file(filePath, std::ios::binary);

    while (true) {
        T entry;
        file.read((char*)&entry, sizeof(T));
        if (file.eof()) break;
        data.push_back(entry);
    }

    file.close();
    return data;
}

template <typename T>
std::vector<T> reader(
    const std::string& filePath,
    int thread_count = std::thread::hardware_concurrency())
{
    std::ifstream file(filePath, std::ios::binary);  
    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return {};
    }

    file.seekg(0, std::ios::end);
    long total_entries = file.tellg() / sizeof(T);
    file.seekg(0, std::ios::beg);

    long chunk_size = total_entries / thread_count;
    long remainder = total_entries % thread_count;

    std::vector<T> combined_data(total_entries);
    std::vector<std::thread> threads;

    long start = 0;
    for (int i = 0; i < thread_count; ++i) {
        long extra = (i < remainder) ? 1 : 0;
        threads.emplace_back(readChunk<T>, filePath, start,
            start + chunk_size + extra, std::ref(combined_data));
        start += chunk_size + extra;
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return combined_data;
}

std::vector<UBIEngine::IO::prev_trade_info> read_prev_trade_info(
    const std::string& filePath,
    int thread_count = std::thread::hardware_concurrency())
{
    return reader<UBIEngine::IO::prev_trade_info>(filePath, thread_count);
}

std::vector<UBIEngine::IO::order_log> read_order_log(
    const std::string& filePath,
    int thread_count = std::thread::hardware_concurrency())
{
    return reader<UBIEngine::IO::order_log>(filePath, thread_count);
}

std::vector<UBIEngine::IO::alpha> read_alpha(
    const std::string& filePath,
    int thread_count = std::thread::hardware_concurrency())
{
    return reader<UBIEngine::IO::alpha>(filePath, thread_count);
}

// For priority_queue IO::twap_order
struct priority_cmp {
    bool operator()(const IO::twap_order& a, const IO::twap_order& b) {
        return a.timestamp > b.timestamp;
    }
};

void emplaceTwapOrder_queue(std::priority_queue<twap_order, std::vector<twap_order>, priority_cmp>& queue,
    const char* instrument_id, long timestamp,
    int32_t direction, int32_t volume, double price)
{
    IO::twap_order order;
    std::memcpy(order.instrument_id, instrument_id, sizeof(order.instrument_id));
    order.timestamp = timestamp;
    order.direction = direction;
    order.volume = volume;
    order.price = price;
    queue.emplace(order);
}


void emplaceTwapOrder_vec(std::vector<twap_order>& vec,
    const char* instrument_id, long timestamp,
    int32_t direction, int32_t volume, double price)
{
    IO::twap_order order;
    std::memcpy(order.instrument_id, instrument_id, sizeof(order.instrument_id));
    order.timestamp = timestamp;
    order.direction = direction;
    order.volume = volume;
    order.price = price;
    vec.emplace_back(order);
}

void emplacePnlAndPos_vec(std::vector<pnl_and_pos>& vec,
    const char* instrument_id, int32_t position, double pnl)
{
    IO::pnl_and_pos pnl_and_pos;
    std::memcpy(pnl_and_pos.instrument_id, instrument_id, sizeof(pnl_and_pos.instrument_id));
    pnl_and_pos.position = position;
    pnl_and_pos.pnl = pnl;
    vec.emplace_back(pnl_and_pos);
}

} // namespace UBIEngine::IO
#endif // UBI_TRADER_IO_READER_H
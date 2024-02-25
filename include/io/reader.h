#ifndef UBI_TRADER_IO_READER_H
#define UBI_TRADER_IO_READER_H
#include <fcntl.h>  // For open().
#include <io/type.h>
#include <sys/mman.h>  // For mmap().
#include <sys/stat.h>  // 包含获取文件状态的函数定义。
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <queue>
#include <thread>
#include <vector>

namespace UBIEngine::IO {

template <typename T>
void readChunk_backup(const std::string& filename, long start, long end, std::vector<T>& data) {
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

template <typename T>
void readChunk(const std::string& filename, long start, long end, std::vector<T>& data) {
    // Use buffer to fast read.
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Error opening file");
    }

    file.seekg(start * sizeof(T), std::ios::beg);
    long readCount = end - start;

    if (readCount > 0) {
        file.read(reinterpret_cast<char*>(&data[start]), readCount * sizeof(T));
        if (!file) {
            throw std::runtime_error("Error reading file");
        }
    }
}

/* No buffer version */
template <typename T>
std::vector<T> reader_sync_backup(const std::string& filePath) {
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

/* Use buffer to fast read */
template <typename T>
std::vector<T> reader_sync(const std::string& filePath) {
    std::vector<T> data;
    std::ifstream file(filePath, std::ios::binary);

    // 计算文件大小
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // 预分配向量空间
    size_t numElements = fileSize / sizeof(T);
    data.reserve(numElements);

    // 确保 bufferSize 是 sizeof(T) 的整数倍
    const size_t bufferSize = 1024 - (1024 % sizeof(T));
    std::vector<char> buffer(bufferSize);

    while (!file.eof()) {
        file.read(buffer.data(), bufferSize);
        size_t bytesRead = file.gcount();

        for (size_t i = 0; i < bytesRead / sizeof(T); ++i) {
            data.push_back(*reinterpret_cast<T*>(buffer.data() + i * sizeof(T)));
        }
    }

    file.close();
    return data;
}

/* RAII class for memory-mapped file */
class MappedFile {
   public:
    MappedFile(const std::string& filePath) {
        // Open the file in read-only mode.
        int fd = open(filePath.c_str(), O_RDONLY);
        if (fd == -1) {
            throw std::runtime_error("Error opening file for reading: " + filePath);
        }

        // Get file status.
        // For `stat` defien refer to:
        // https://pubs.opengroup.org/onlinepubs/009696699/basedefs/sys/stat.h.html.
        struct stat sb;  // sb means status buffer.
        if (fstat(fd, &sb) == -1) {
            close(fd);
            throw std::runtime_error("Error getting file status: " + filePath);
        }

        // Get the file size.
        length = sb.st_size;
        std::cout << "File size: " << length << std::endl;

        // Use the mmap system call to map the file into memory.
        addr = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
        if (addr == MAP_FAILED) {
            close(fd);
            throw std::runtime_error("Error mapping file into memory: " + filePath);
        }
    }

    ~MappedFile() {
        if (addr != nullptr) {
            munmap(addr, length);
        }
        if (fd != -1) {
            close(fd);
        }
    }

    void* data() const { return addr; }

    size_t size() const { return length; }

    MappedFile(const MappedFile&) = delete;             // Forbid Copy constructor
    MappedFile& operator=(const MappedFile&) = delete;  // Forbid Copy assignment

   private:
    int fd = -1;
    void* addr = nullptr;
    size_t length = 0;
};

template <typename T>
std::vector<T> reader_mmap(const std::string& filePath) {
    // Use `MappedFile` class to map the file into memory.
    MappedFile mappedFile(filePath);

    size_t size = mappedFile.size();        // Get the size of the mapped file.
    size_t numElements = size / sizeof(T);  // Get the number of elements in the file.

    // Get a pointer to the mapped data.
    T* dataPtr = static_cast<T*>(mappedFile.data());

    // Copy the mapped data into a vector.
    std::vector<T> data(dataPtr, dataPtr + numElements);

    return data;
}

template <typename T>
std::vector<T> reader(const std::string& filePath,
                      int thread_count = std::thread::hardware_concurrency()) {
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
        threads.emplace_back(readChunk<T>, filePath, start, start + chunk_size + extra,
                             std::ref(combined_data));
        start += chunk_size + extra;
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return combined_data;
}

std::vector<UBIEngine::IO::prev_trade_info> read_prev_trade_info(
    const std::string& filePath, int thread_count = std::thread::hardware_concurrency()) {
    return reader<UBIEngine::IO::prev_trade_info>(filePath, thread_count);
}

std::vector<UBIEngine::IO::order_log> read_order_log(
    const std::string& filePath, int thread_count = std::thread::hardware_concurrency()) {
    return reader<UBIEngine::IO::order_log>(filePath, thread_count);
}

std::vector<UBIEngine::IO::alpha> read_alpha(
    const std::string& filePath, int thread_count = std::thread::hardware_concurrency()) {
    return reader<UBIEngine::IO::alpha>(filePath, thread_count);
}

// For priority_queue IO::twap_order
struct priority_cmp {
    bool operator()(const IO::twap_order& a, const IO::twap_order& b) {
        return a.timestamp > b.timestamp;
    }
};

void emplaceTwapOrder_queue(
    std::priority_queue<twap_order, std::vector<twap_order>, priority_cmp>& queue,
    const char* instrument_id, long timestamp, int32_t direction, int32_t volume, double price) {
    IO::twap_order order;
    std::memcpy(order.instrument_id, instrument_id, sizeof(order.instrument_id));
    order.timestamp = timestamp;
    order.direction = direction;
    order.volume = volume;
    order.price = price;
    queue.emplace(order);
}

void emplaceTwapOrder_vec(std::vector<twap_order>& vec, const char* instrument_id, long timestamp,
                          int32_t direction, int32_t volume, double price) {
    IO::twap_order order;
    std::memcpy(order.instrument_id, instrument_id, sizeof(order.instrument_id));
    order.timestamp = timestamp;
    order.direction = direction;
    order.volume = volume;
    order.price = price;
    vec.emplace_back(order);
}

void emplacePnlAndPos_vec(std::vector<pnl_and_pos>& vec, const char* instrument_id,
                          int32_t position, double pnl) {
    IO::pnl_and_pos pnl_and_pos;
    std::memcpy(pnl_and_pos.instrument_id, instrument_id, sizeof(pnl_and_pos.instrument_id));
    pnl_and_pos.position = position;
    pnl_and_pos.pnl = pnl;
    vec.emplace_back(pnl_and_pos);
}

}  // namespace UBIEngine::IO
#endif  // UBI_TRADER_IO_READER_H
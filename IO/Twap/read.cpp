#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

struct twap_order {
    char instrument_id[8];
    long timestamp;
    int direction;
    int volume;
    double price;
} __attribute__((packed));


void print_order_log(const std::vector<twap_order>& data, int count = -1) {
    int cur_count = 0;
    for (const auto& entry : data) {
        std::cout << "Instrument ID: " << entry.instrument_id
                  << ", Timestamp: " << entry.timestamp
                  << ", Direction: " << entry.direction
                  << ", Volume: " << entry.volume
                  << ", Price: " << entry.price
                  << std::endl;
        ++ cur_count;
        if (cur_count >= count && count != -1) break;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_file>" << std::endl;
        return 1;
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    std::ifstream file(argv[1], std::ios::binary);  // 使用命令行参数提供的文件名
    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    std::vector<twap_order> data;

    while (true) {
        twap_order entry;
        file.read((char*)&entry, sizeof(twap_order));
        if (file.eof()) break;
        data.push_back(entry);
    }

    file.close();
    auto end_time = std::chrono::high_resolution_clock::now();

    std::cout << "Read [Serial] Time taken: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
        << "ms" << std::endl;
    print_order_log(data);

    return 0;
}


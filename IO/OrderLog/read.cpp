#include <iostream>
#include <fstream>
#include <vector>

struct order_log {
    char instrument_id[8];
    long timestamp;
    int type;
    int direction;
    int volume;
    double price_off;
} __attribute__((packed));

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);  // 使用命令行参数提供的文件名
    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    std::vector<order_log> data;

    while (true) {
        order_log entry;
        file.read((char*)&entry, sizeof(order_log));
        if (file.eof()) break;
        data.push_back(entry);
    }

    file.close();

    for (const auto& entry : data) {
        std::cout << "Instrument ID: " << entry.instrument_id
                  << ", Timestamp: " << entry.timestamp
                  << ", Type: " << entry.type
                  << ", Direction: " << entry.direction
                  << ", Volume: " << entry.volume
                  << ", Price Offset: " << entry.price_off
                  << std::endl;
    }
    std::cout << "Please press enter to continue..." << std::endl;
    std::cin.get();

    return 0;
}


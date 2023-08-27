#include <iostream>
#include <fstream>
#include <vector>

struct prev_trade_info {
    char instrument_id[8];
    double prev_close_price;
    int prev_position;
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

    std::vector<prev_trade_info> data;

    while (true) {
        prev_trade_info entry;
        file.read((char*)&entry, sizeof(prev_trade_info));
        if (file.eof()) break;
        data.push_back(entry);
    }

    file.close();

    for (const auto& entry : data) {
        std::cout << "Instrument ID: " << entry.instrument_id
                  << ", Previous Close Price: " << entry.prev_close_price
                  << ", Previous Position: " << entry.prev_position << std::endl;
    }

    return 0;
}


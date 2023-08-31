#ifndef UBI_TRADER_IO_TYPE_H
#define UBI_TRADER_IO_TYPE_H
#include <vector>

namespace UBIEngine::IO {
struct prev_trade_info {
    char instrument_id[8];
    double prev_close_price;
    int prev_position;
} __attribute__((packed));

struct order_log {
    char instrument_id[8];
    long timestamp;
    int type;
    int direction;
    int volume;
    double price_off;
} __attribute__((packed));

struct alpha {
    char instrument_id[8];
    long timestamp;
    int target_volume;
} __attribute__((packed));

struct twap_order {
    char instrument_id[8];
    long timestamp;
    int direction;
    int volume;
    double price;
} __attribute__((packed));

struct pnland_pos {
    char instrument_id[8];
	int position;
	double pnl;
} __attribute__((packed));

template<typename T>
class VectorQueue {
private:
    std::vector<T> data;
    size_t frontIndex = 0;  // 这是队列的“头部”

public:
    // 使用已存在的vector来构造
    VectorQueue(std::vector<T>&& vec) : data(std::move(vec)) {}

    // 判断队列是否为空
    bool empty() const {
        return frontIndex >= data.size();
    }

    // 获取队列的头部元素
    T& front() {
        if (empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return data[frontIndex];
    }

    const T& front() const {
        if (empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return data[frontIndex];
    }

    // 出队操作
    void pop() {
        if (empty()) {
            throw std::runtime_error("Queue is empty");
        }
        ++frontIndex;
    }

    // 入队操作
    void push(const T& value) {
        data.push_back(value);
    }

    void push(T&& value) {
        data.push_back(std::move(value));
    }
};

} // namespace UBIEngine::IO
#endif // UBI_TRADER_IO_TYPE_H
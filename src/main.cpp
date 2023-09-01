#include "concurrent_market.h"
#include "market.h"
#include "io/reader.h"
#include "io/sender.h"
#include "utils/sort.h"
#include "symbol.h"
#include "robin_hood.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <chrono>

using namespace UBIEngine;

// 假设 IO::pnl_and_pos 和 IO::twap_order 都有大小信息
bool send_data(
    const IO::GlobalSocket& gSocket,
    const std::string& filename,   // <-- File name to be sent
    const std::vector<IO::pnl_and_pos>& pnls,
    const std::vector<IO::twap_order>& ans)
{
    int sockfd = gSocket.getSocket();

    // Parse and Send the filename
    int year, month, day, x, y;
    sscanf(filename.c_str(), "%4d%2d%2d_%d_%d", &year, &month, &day, &x, &y);
    std::cout << "filename: " << filename << std::endl;
    std::cout << "year: " << year << " month: " << month << " day: "
        << day << " x: " << x << " y: " << y << std::endl;
    ssize_t bytesWritten = write(sockfd, &year, sizeof(year));
    if (bytesWritten != sizeof(year)) {
        return false;
    }
    bytesWritten = write(sockfd, &month, sizeof(month));
    if (bytesWritten != sizeof(month)) {
        return false;
    }
    bytesWritten = write(sockfd, &day, sizeof(day));
    if (bytesWritten != sizeof(day)) {
        return false;
    }
    bytesWritten = write(sockfd, &x, sizeof(x));
    if (bytesWritten != sizeof(x)) {
        return false;
    }
    bytesWritten = write(sockfd, &y, sizeof(y));
    if (bytesWritten != sizeof(y)) {
        return false;
    }

    // Send pnls
    ssize_t bytesToSend = pnls.size() * sizeof(IO::pnl_and_pos);
    bytesWritten = write(sockfd, pnls.data(), bytesToSend);
    if (bytesWritten != bytesToSend) {
        return false;
    }
    std::cout << "Successfully sent pnls!" << std::endl;

    // Send ans
    bytesToSend = ans.size() * sizeof(IO::twap_order);
    bytesWritten = write(sockfd, ans.data(), bytesToSend);
    if (bytesWritten != bytesToSend) {
        return false;
    }
    std::cout << "Successfully sent ans!" << std::endl;

    return true;
}

std::future<bool> async_send_data(
    const IO::GlobalSocket& gSocket,
    const std::string& filename,   // <-- File name to be sent
    const std::vector<IO::pnl_and_pos>& pnls,
    const std::vector<IO::twap_order>& ans)
{
    return std::async(std::launch::async, [&]() {
        int sockfd = gSocket.getSocket();

        // Parse and Send the filename
        int year, month, day, x, y;
        sscanf(filename.c_str(), "%4d%2d%2d_%d_%d", &year, &month, &day, &x, &y);
        std::cout << "filename: " << filename << std::endl;
        std::cout << "year: " << year << " month: " << month << " day: "
            << day << " x: " << x << " y: " << y << std::endl;
        ssize_t bytesWritten = write(sockfd, &year, sizeof(year));
        if (bytesWritten != sizeof(year)) {
            return false;
        }
        bytesWritten = write(sockfd, &month, sizeof(month));
        if (bytesWritten != sizeof(month)) {
            return false;
        }
        bytesWritten = write(sockfd, &day, sizeof(day));
        if (bytesWritten != sizeof(day)) {
            return false;
        }
        bytesWritten = write(sockfd, &x, sizeof(x));
        if (bytesWritten != sizeof(x)) {
            return false;
        }
        bytesWritten = write(sockfd, &y, sizeof(y));
        if (bytesWritten != sizeof(y)) {
            return false;
        }

        // Send pnls
        ssize_t bytesToSend = pnls.size() * sizeof(IO::pnl_and_pos);
        bytesWritten = write(sockfd, pnls.data(), bytesToSend);
        if (bytesWritten != bytesToSend) {
            return false;
        }
        std::cout << "Successfully sent pnls!" << std::endl;

        // Send ans
        bytesToSend = ans.size() * sizeof(IO::twap_order);
        bytesWritten = write(sockfd, ans.data(), bytesToSend);
        if (bytesWritten != bytesToSend) {
            return false;
        }
        std::cout << "Successfully sent ans!" << std::endl;

        return true;
    });
}

template<typename T>
bool writeToFile(const std::vector<T>& data, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary);
    
    if (!outFile.is_open()) {
        std::cerr << "Error: Couldn't open the file: " << filename << std::endl;
        return false;
    }

    outFile.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));
    outFile.close();
    return true;
}

// void solve(std::string& dataset_dir_path, IO::GlobalSocket &gSocket) {
void solve(std::string& dataset_dir_path) {
    /* Data Pre-Process */
    // auto order_logs = IO::read_order_log(dataset_dir_path + "/order_log");
    auto order_logs = IO::reader_sync<IO::order_log>(dataset_dir_path + "/order_log");
    auto order_queue = IO::VectorQueue<IO::order_log>(std::move(order_logs));
    // auto alpha_arr = IO::read_alpha(dataset_dir_path + "/alpha");
    auto alpha_arr = IO::reader_sync<IO::alpha>(dataset_dir_path + "/alpha");
    auto alpha_queue = IO::VectorQueue<IO::alpha>(std::move(alpha_arr));
    auto prev_trade_infos = IO::reader_sync<IO::prev_trade_info>(
        dataset_dir_path + "/prev_trade_info");
    std::priority_queue<IO::twap_order, std::vector<IO::twap_order>, IO::priority_cmp> strategy_queue;


    std::vector<uint32_t> session_nums = {3, 3, 3, 5, 5};
    std::vector<uint32_t> session_lengths = {1, 3, 5, 2, 3};


    for (int i = 0; i < session_nums.size(); ++ i) {
        uint32_t session_num = session_nums[i];
        uint32_t session_length = session_lengths[i];
        assert (strategy_queue.size() == 0
            && "Strategy queue should be empty at the beginning!");

        std::cout << "Start init symbol_manager and market!\n";
        /* Init market */
        Market market;
        // ConcurrentMarket market(1);
        SymbolManager symbol_manager;
        for (const auto &prev_info : prev_trade_infos) {
            // 首先这样可以获得 char[8] to uint32_t 的转换。
            symbol_manager.getSymbolId(prev_info.instrument_id);
            // 把对应的 symbol 加入到 market 中。
            market.addSymbol(
                symbol_manager.getSymbolId(prev_info.instrument_id),
                std::string(prev_info.instrument_id),
                static_cast<uint64_t>(prev_info.prev_close_price * 100 + 0.5),
                prev_info.prev_position);
        }


        uint64_t order_id = 0;
        // 为了方便，直接分配好内存。
        std::vector<IO::twap_order> ans;
        ans.reserve(alpha_arr.size() * session_num);
        int32_t symbol_id_tar = 0x3f3f3f3f;
        uint64_t cnt = 0;
        std::cout << "[Start]: "
            << session_num << "_" << session_length << std::endl;
        auto start_time = std::chrono::steady_clock::now();
        /* main loop */
        while (!order_queue.empty()
            || !alpha_queue.empty()
            || !strategy_queue.empty())
        {
            // 即使时间相同，也要先处理order_log。
            if (!order_queue.empty() && 
                (alpha_queue.empty() || order_queue.front().timestamp <= alpha_queue.front().timestamp) &&
                (strategy_queue.empty() || order_queue.front().timestamp <= strategy_queue.top().timestamp))
            {
                const auto& raw_order = order_queue.front();
                order_queue.pop();
                uint32_t symbol_id = symbol_manager.getSymbolId(raw_order.instrument_id);
                OrderSide side = raw_order.direction == 1 ? OrderSide::Bid : OrderSide::Ask;
                //! 获取基准价格
                auto basePrice = market.getBasePrice(symbol_id, side);
                double price_off_100 = raw_order.price_off * 100;
                int32_t price_off_int;
                if (price_off_100 > 0)
                    price_off_int = static_cast<int32_t> (price_off_100 + 0.5);
                else
                    price_off_int = static_cast<int32_t> (price_off_100 - 0.5);

                //! Only LIMIT order has price_off.
                if (raw_order.type != 0) price_off_int = 0;
                
                uint32_t price = raw_order.type == 0 ? basePrice + price_off_int : 0;
                Order order = Order::newOrder(
                    int2OrderType(raw_order.type),
                    side,
                    order_id++,
                    symbol_id,
                    raw_order.volume,
                    price,
                    false // is Strategy Order?
                );

                // ============================================================================================================
                if (symbol_id == symbol_id_tar || symbol_id_tar == -1) {
                    double price_t = static_cast<double>(price) / 100;
                    double basePrice_t = static_cast<double>(basePrice) / 100;
                    double down_limit = static_cast<double>(market.getDownLimit(symbol_id, side)) / 100;
                    double up_limit = static_cast<double>(market.getUpLimit(symbol_id, side)) / 100;
                    std::cout << "Symbol_id: " << symbol_id << std::endl;
                    if (raw_order.type == 0)
                        printf("history order: timestamp=%d, direction=%d, order_type=%d, volume=%d, price=%.6lf, base_price=%.6lf, up_limit=%.6lf, down_limit=%.6lf\n",
                            raw_order.timestamp, raw_order.direction, raw_order.type, raw_order.volume, price_t, basePrice_t, up_limit, down_limit);
                    else 
                        printf("history order: timestamp=%d, direction=%d, order_type=%d, volume=%d\n",
                            raw_order.timestamp, raw_order.direction, raw_order.type, raw_order.volume);
                }
                // ============================================================================================================

                // Check 下价格是否合法
                if (price_off_int < 0 && basePrice < -price_off_int) {
                    std::cout << ">>>>>>>>>> Price off is too large! <<<<<<<<<<\n";
                    continue;
                }

                market.addOrder(order);
            }
            // 处理信号
            if (!alpha_queue.empty() &&
                (order_queue.empty() || alpha_queue.front().timestamp < order_queue.front().timestamp) &&
                (strategy_queue.empty() || alpha_queue.front().timestamp <= strategy_queue.top().timestamp))
            {
                auto alpha_t = alpha_queue.front();
                alpha_queue.pop();

                uint32_t symbol_id = symbol_manager.getSymbolId(alpha_t.instrument_id);
                const auto& pnl_helper = market.getPnlHelper(symbol_id);
                int32_t diff = alpha_t.target_volume - pnl_helper.getPosition();

                //! If diff == 0, skip!!
                if (diff == 0) continue;

                // 通过 diff 判断是 Bid or Ask
                int32_t strategy_side = diff > 0 ? 1 : -1;
                // 并且下单的量肯定是 diff 的绝对值
                uint64_t volume = std::abs(diff);

                for (int part_id = 0; part_id < session_num; ++ part_id) {
                    int32_t part_volume = (volume * (part_id + 1) / session_num) - (volume * part_id / session_num);
                    // if (symbol_id == 1)
                        // std::cout << "timestap: " << alpha_t.timestamp + part_id * session_length * 1000 << " part_volume: " << part_volume
                        // << " queue_size: " << strategy_queue.size() << 
                        // << " front: " << 
                    IO::emplaceTwapOrder_queue(strategy_queue,
                        alpha_t.instrument_id,
                        alpha_t.timestamp + part_id * session_length * 1000,
                        strategy_side,
                        part_volume,
                        static_cast<double>(0) // ! Note 需要就地构造
                    );
                }
            }

            // 处理策略单
            if (!strategy_queue.empty() &&
                (order_queue.empty() || strategy_queue.top().timestamp < order_queue.front().timestamp) &&
                (alpha_queue.empty() || strategy_queue.top().timestamp < alpha_queue.front().timestamp))
            {
                auto strategy_order = strategy_queue.top();
                strategy_queue.pop();

                uint32_t symbol_id = symbol_manager.getSymbolId(strategy_order.instrument_id);
                OrderSide side = strategy_order.direction == 1 ? OrderSide::Bid : OrderSide::Ask;
                //! 获取基准价格.
                auto basePrice = market.getBasePrice(symbol_id, side);

                // Must be LIMIT order.
                uint32_t price = basePrice;
                double price_double = static_cast<double>(price) / 100;

                //TODO 写到 ans 数组中
                IO::emplaceTwapOrder_vec(ans,
                    strategy_order.instrument_id,
                    strategy_order.timestamp,
                    strategy_order.direction,
                    strategy_order.volume,
                    price_double
                );

                // ========================================================================
                // if (symbol_id == symbol_id_tar || symbol_id_tar == -1) {
                //     printf("twap order: timestamp=%d, direction=%d, volume=%d, price=%.6lf\n",
                //             strategy_order.timestamp,
                //             strategy_order.direction,
                //             strategy_order.volume,
                //             price_double);
                // }
                // ========================================================================
                // 如果 volume == 0, 则不进入撮合系统.
                if (strategy_order.volume == 0) continue;
            
                Order order = Order::newOrder(
                    OrderType::LIMIT,
                    side,
                    order_id++,
                    symbol_id,
                    strategy_order.volume,
                    price,
                    true // is Strategy Order?
                );
                market.addOrder(order);
            }
            // ++cnt;
            // if (cnt >= 20000) break;
        }

        auto end_time = std::chrono::steady_clock::now();
        std::cout << "[Done]: "
            << session_num << "_" << session_length
            << " Cost Time: " << 
            static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()) / 1000
            << "s" << std::endl;

        std::vector<IO::pnl_and_pos> pnls;
        for (const auto &prev_info : prev_trade_infos) {
            // 首先这样可以获得 char[8] to uint32_t 的转换。
            int64_t pnl = market.calculatePnl(symbol_manager.getSymbolId(prev_info.instrument_id));
            double pnl_double = static_cast<double>(pnl) / 100;
            IO::emplacePnlAndPos_vec(pnls,
                prev_info.instrument_id,
                market.getPnlHelper(symbol_manager.getSymbolId(prev_info.instrument_id)).getPosition(),
                pnl_double
            );
        }

        /* 排序后写到本地 */
        Utils::multiThreadSort(ans, Utils::my_compare_twap);
        Utils::multiThreadSort(pnls, Utils::my_compare_pnl);
        std::string target_twap_name = "/home/team5/output/twap_order/20160202_" +
            std::to_string(session_num) + "_" + std::to_string(session_length);
        std::string target_pnl_name = "/home/team5/output/pnl_and_position/20160202_" +
            std::to_string(session_num) + "_" + std::to_string(session_length);
        writeToFile<IO::twap_order>(ans, target_twap_name);
        writeToFile<IO::pnl_and_pos>(pnls, target_pnl_name);
        std::string file_name_ = "20160202_" +
            std::to_string(session_num) + "_" + std::to_string(session_length);

        // auto gSocket = IO::GlobalSocket("172.28.142.142", 8081);
        // auto finish = async_send_data(gSocket, file_name_, pnls, ans);
        // finish.wait();
        // auto finish = send_data(gSocket, file_name_, pnls, ans);
        order_queue.reset();
        alpha_queue.reset();
        while (!strategy_queue.empty()) strategy_queue.pop();
    }
}

int main() {
    std::string dataset_dir_path = "/mnt/data/20180404";
    // solve(dataset_dir_path, gSocket);
    solve(dataset_dir_path);
    return 0;
}
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

struct order {
    char instrument_id[8];
    long timestamp;
    int type;
    int volume;
    double price;
} __attribute__((packed));

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;

    std::cout << "开始创建 socket..." << std::endl;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "创建 socket 失败！" << std::endl;
        return 1;
    }
    std::cout << "socket 创建成功。" << std::endl;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "172.28.142.142", &serv_addr.sin_addr);

    std::cout << "尝试连接到服务器 172.28.142.142:8080..." << std::endl;
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        std::cerr << "连接服务器失败！" << std::endl;
        close(sockfd);
        return 1;
    }
    std::cout << "成功连接到服务器。" << std::endl;

    order ord;
    strcpy(ord.instrument_id, "BTCUSD");
    ord.timestamp = 1630000000;
    ord.type = 1;
    ord.volume = 100;
    ord.price = 50000.0;

    std::cout << "发送数据到服务器..." << std::endl;
    ssize_t bytesWritten = write(sockfd, &ord, sizeof(ord));
    if (bytesWritten != sizeof(ord)) {
        std::cerr << "发送数据失败或数据不完整！" << std::endl;
        close(sockfd);
        return 1;
    }
    std::cout << "数据发送成功。" << std::endl;

    close(sockfd);
    std::cout << "关闭连接。" << std::endl;

    return 0;
}

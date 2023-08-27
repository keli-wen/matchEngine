#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstring>

struct order {
    char instrument_id[8];
    long timestamp;
    int type;
    int volume;
    double price;
} __attribute__((packed));

int main() {
    int sockfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    // 创建 socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        std::cerr << "创建 socket 失败！" << std::endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);

    // 绑定地址和端口
    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        std::cerr << "绑定端口失败！" << std::endl;
        close(sockfd);
        return 1;
    }

    // 开始监听
    listen(sockfd, 5);
    connfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
    if(connfd == -1) {
        std::cerr << "连接接受失败！" << std::endl;
        close(sockfd);
        return 1;
    }

    order ord;
    ssize_t bytesRead = read(connfd, &ord, sizeof(ord));
    if(bytesRead != sizeof(ord)) {
        std::cerr << "读取数据失败或数据不完整！" << std::endl;
        close(connfd);
        close(sockfd);
        return 1;
    }

    std::ofstream outfile("/home/team5/order.dat", std::ios::binary);
    if(!outfile.is_open()) {
        std::cerr << "打开文件失败！" << std::endl;
        close(connfd);
        close(sockfd);
        return 1;
    }

    outfile.write((char*)&ord, sizeof(ord));
    outfile.close();

    std::cout << "数据接收成功并保存到文件！" << std::endl;

    close(connfd);
    close(sockfd);
    return 0;
}

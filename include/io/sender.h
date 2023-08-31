#ifndef UBI_TRADER_IO_SENDER_H
#define UBI_TRADER_IO_SENDER_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <io/type.h>

namespace UBIEngine::IO {
class GlobalSocket {
private:
    int sockfd;
    struct sockaddr_in serv_addr;

public:
    GlobalSocket(const char* ip, int port) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            throw std::runtime_error("创建 socket 失败！");
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &serv_addr.sin_addr);

        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
            close(sockfd);
            throw std::runtime_error("连接服务器失败！");
        }
    }

    ~GlobalSocket() {
        close(sockfd);
    }

    int getSocket() const {
        return sockfd;
    }
};
} // namespace UBIEngine::IO
#endif
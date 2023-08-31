#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>

namespace IO {
struct twap_order {
    char instrument_id[8];
    long timestamp;
    int direction;
    int volume;
    double price;
} __attribute__((packed));

struct pnl_and_pos {
    char instrument_id[8];
	int position;
	double pnl;
} __attribute__((packed));
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
    std::cout << "Done!" << std::endl;
    return true;
}

class GlobalReceiver {
private:
    int server_sockfd;
    int client_sockfd;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

public:
    GlobalReceiver(int port) {
        server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sockfd == -1) {
            throw std::runtime_error("创建 socket 失败！");
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
            close(server_sockfd);
            throw std::runtime_error("Bind failed!");
        }

        listen(server_sockfd, 5);
    }

    ~GlobalReceiver() {
        close(client_sockfd);
        close(server_sockfd);
    }

    void waitForConnection() {
        while (true) {  // Infinite loop to keep accepting connections
            std::cout << "Waiting for connection..." << std::endl;
            socklen_t client_len = sizeof(client_addr);
            client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &client_len);
            if (client_sockfd == -1) {
                std::cerr << "Accept failed!" << std::endl;
                continue;  // Go back to waiting for the next connection
            }
            std::cout << "Connected to a client!" << std::endl;

            try {
                // Call a function or loop here to handle this particular client's requests
                handleClient();
            } catch (const std::exception& e) {
                std::cerr << "Error handling client: " << e.what() << std::endl;
            }

            close(client_sockfd);  // Close this client's socket and wait for another
        }
    }

    void handleClient() {
        while (true) { // Loop to continuously receive data from this client
            // Receive filename
            int year, month, day, x, y;
            
            ssize_t bytesRead = read(client_sockfd, &year, sizeof(year));
            if (bytesRead != sizeof(year)) {
                break;  // Connection error or client might have closed the connection
            }
            
            bytesRead = read(client_sockfd, &month, sizeof(month));
            if (bytesRead != sizeof(month)) {
                break;
            }
            
            bytesRead = read(client_sockfd, &day, sizeof(day));
            if (bytesRead != sizeof(day)) {
                break;
            }
            
            bytesRead = read(client_sockfd, &x, sizeof(x));
            if (bytesRead != sizeof(x)) {
                break;
            }
            
            bytesRead = read(client_sockfd, &y, sizeof(y));
            if (bytesRead != sizeof(y)) {
                break;
            }

            char filename[64];  // large enough buffer to store the filename
            snprintf(filename, sizeof(filename), "%04d%02d%02d_%d_%d", year, month, day, x, y);
            std::cout << "Year: " << year << std::endl;
            std::cout << "Month: " << month << std::endl;
            std::cout << "Day: " << day << std::endl;
            std::cout << "X: " << x << std::endl;
            std::cout << "Y: " << y << std::endl;

            char filename_true[64];
            sprintf(filename_true, "%04d%02d%02d_%d_%d", year, month, day, x, y);

            std::cout << "Received filename: " << filename_true << std::endl;

            // Receive pnls and ans as usual
            std::vector<IO::pnl_and_pos> pnls = receiveData<IO::pnl_and_pos>();
            std::vector<IO::twap_order> ans = receiveData<IO::twap_order>();

            std::string filename_str = std::string(filename_true);
            std::string filename_twap = "/home/team5/output/twap_order/" + filename_str;
            std::string filename_pnl = "/home/team5/output/pnl_and_pos/" + filename_str;
            writeToFile<IO::twap_order>(ans, filename_twap);
            writeToFile<IO::pnl_and_pos>(pnls, filename_pnl);

            
            // Here you can process the received data and filename as needed.
        }
    }

    template <typename T>
    std::vector<T> receiveData() {
        std::vector<T> dataVec;
        T dataItem;
        ssize_t bytesReceived = read(client_sockfd, &dataItem, sizeof(T));
        while (bytesReceived == sizeof(T)) {  // Loop to receive all data of type T
            dataVec.push_back(dataItem);
            bytesReceived = read(client_sockfd, &dataItem, sizeof(T));
        }
        return dataVec;
    }
};

int main() {
    try {
        GlobalReceiver receiver(8080);
        receiver.waitForConnection();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

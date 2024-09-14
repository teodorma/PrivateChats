#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <system_error>
#include <thread>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <condition_variable>
#include "Worker.h"

class Server {
public:
    explicit Server(int PORT);
    ~Server();
    void run();
    void ConnectClient(const std::string &phone_number, const std::string &aes_key, int client_socket);
    bool SendMessage(const std::string &phone_number, const std::string &message);
    static void SendResponse(int client_socket, const std::string &response);
    void removeClient(int client_sock);

private:
    int SOCKET;
    sockaddr_in SERVER_ADDR{};
    static std::unordered_map<std::string, std::pair<int, std::string>>  clients;
    std::vector<int> client_sockets;
    std::mutex clients_mutex;
    std::vector<std::thread> workers;
    std::condition_variable cv;
    std::mutex cv_m;

    void acceptConnections();
};

#endif // SERVER_H

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
#include <map>
#include <mutex>
#include <condition_variable>
#include "Worker.h"

class Server {
public:
    explicit Server(int PORT);
    ~Server();
    void run();
    void RegisterClient(const std::string &phone_number, int client_socket);
    void SendMessage(const std::string &phone_number, const std::string &message);
    static void SendResponse(int client_socket, const std::string &response);

private:
    int SOCKET;
    sockaddr_in SERVER_ADDR{};
    std::map<std::string, int> clients; // Mapping of phone number to socket
    std::mutex clients_mutex; // Mutex to protect access to clients map
    std::vector<std::thread> workers;
    std::condition_variable cv;
    std::mutex cv_m;
    std::vector<int> client_sockets;

    void acceptConnections();
};

#endif // SERVER_H

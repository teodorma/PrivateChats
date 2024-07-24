#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <system_error>
#include <thread>
#include "../Requests/Requests.h"

class Server {
public:
    explicit Server(int PORT);
    ~Server();
    [[nodiscard]] int get_SOCKET() const;
    [[nodiscard]] sockaddr_in get_SERVER_ADDR() const;
    static void Receive(int client_socket);

private:
    int SOCKET;
    sockaddr_in SERVER_ADDR{};
};

#endif // SERVER_H

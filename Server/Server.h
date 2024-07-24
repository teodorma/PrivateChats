//
// Created by maciucateodor on 6/24/24.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>
#include "../Requests/Requests.h"

class Server {
private:
    int SOCKET;
    sockaddr_in SERVER_ADDR{};
public:
    explicit Server(int PORT);

    [[nodiscard]] int get_SOCKET() const;
    [[nodiscard]] sockaddr_in get_SERVER_ADDR() const;

    static void Receive(int client_socket);

    ~Server();
};
#endif //SERVER_SERVER_H

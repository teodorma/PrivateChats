//
// Created by maciucateodor on 6/24/24.
//

#include "Server.h"

Server::Server(const int PORT) {
    try {
        // Build socket
        Server::SOCKET = socket(AF_INET, SOCK_STREAM, 0);
        if (SOCKET == -1) {
            throw std::system_error(errno, std::generic_category());
        }

        // Build socket address binding it to specified port
        this->SERVER_ADDR.sin_family = AF_INET;
        this->SERVER_ADDR.sin_addr.s_addr = INADDR_ANY;
        this->SERVER_ADDR.sin_port = htons(PORT);

        if (bind(SOCKET, (struct sockaddr*)&SERVER_ADDR, sizeof(SERVER_ADDR)) == -1) {
            throw std::system_error(errno, std::generic_category());
        }
    }
    catch (std::system_error& error) {
        std::cout << "Caught system error: \n"
                  << error.what();
    }
}

[[nodiscard]] int Server::get_SOCKET() const{
    return SOCKET;
}

[[nodiscard]] sockaddr_in Server::get_SERVER_ADDR() const {
    return SERVER_ADDR;
}

void Server::Receive(int client_socket) {
    char buff[256] = {0};
    auto n = recv(client_socket, buff, sizeof(buff) - 1, 0);

    if(n < 0){
        std::cerr << "Error reading from socket: " << strerror(errno);
        return;
    }

    buff[n] = '\0';
    std::istringstream stream(buff);
    auto request = new Requests(stream);

    request->Process();

    close(client_socket);
}

Server::~Server() {
    close(Server::SOCKET);
}
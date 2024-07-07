//
// Created by maciucateodor on 6/24/24.
//

#include "Server.h"

Server::Server(const int PORT) {
    try {
        Server::SOCKET = socket(AF_INET, SOCK_STREAM, 0);
        if (SOCKET == -1) {
            throw std::system_error(errno, std::generic_category());
        }

        this->SERVER_ADDR.sin_family = AF_INET;
        this->SERVER_ADDR.sin_addr.s_addr = INADDR_ANY;
        this->SERVER_ADDR.sin_port = htons(PORT);

        if (bind(SOCKET, (struct sockaddr*)&SERVER_ADDR, sizeof(SERVER_ADDR)) == -1) {
            throw std::system_error(errno, std::generic_category());
        }
    }
    catch (std::system_error& error) {
        std::cout << "Caught system error: "
                  << error.code()
                  << error.what()
                  << errno;
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
    auto request = Requests(stream);
    Json::FastWriter WRITER;
    auto RESPONSE = WRITER.write(request.Process());

    std::strncpy(buff, RESPONSE.c_str(), sizeof(buff) - 1);
    n = write(client_socket, buff, sizeof(buff)-1);

    if(n < 0){
        std::cerr << "Error sending response: " << strerror(errno);
        return;
    }

    close(client_socket);
}

Server::~Server() {
    close(Server::SOCKET);
}
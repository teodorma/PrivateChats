#include "Server.h"

Server::Server(int PORT) {
    try {
        SOCKET = socket(AF_INET, SOCK_STREAM, 0);
        if (SOCKET == -1) {
            throw std::system_error(errno, std::generic_category());
        }

        SERVER_ADDR.sin_family = AF_INET;
        SERVER_ADDR.sin_addr.s_addr = INADDR_ANY;
        SERVER_ADDR.sin_port = htons(PORT);

        if (bind(SOCKET, (struct sockaddr*)&SERVER_ADDR, sizeof(SERVER_ADDR)) == -1) {
            throw std::system_error(errno, std::generic_category());
        }
    } catch (const std::system_error& error) {
        std::cerr << "Caught system error: " << error.code() << "\n"
                  << error.what() << "\n"
                  << errno << "\n";
    }
}

[[nodiscard]] int Server::get_SOCKET() const {
    return SOCKET;
}

[[nodiscard]] sockaddr_in Server::get_SERVER_ADDR() const {
    return SERVER_ADDR;
}

void Server::Receive(int client_socket) {
    char buff[4096] = {0};
    auto n = recv(client_socket, buff, sizeof(buff) - 1, 0);

    if (n < 0) {
        std::cerr << "Error reading from socket: " << strerror(errno) << std::endl;
        close(client_socket);
        return;
    }

    buff[n] = '\0';
    std::istringstream stream(buff);
    auto request = Requests(stream);
    auto RESPONSE = request.Process();

    n = send(client_socket, RESPONSE.c_str(), RESPONSE.size(), 0);
    if (n < 0) {
        std::cerr << "Error sending response: " << strerror(errno) << std::endl;
    }

    close(client_socket);
}

Server::~Server() {
    close(SOCKET);
}

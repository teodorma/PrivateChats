#include "Worker.h"
#include "Server.h"
#include "../Requests/Requests.h"

Worker::Worker(int client_socket, Server &server) : client_socket(client_socket), server(server) {}

void Worker::operator()() {
    handleClient();
}

void Worker::handleClient() {
    std::cout << "Started handling client socket " << client_socket << std::endl;

    while (true) {
        char buff[4096] = {0};
        auto n = recv(client_socket, buff, sizeof(buff) - 1, 0);

        if (n < 0) {
            std::cerr << "Error reading from socket: " << strerror(errno) << std::endl;
            close(client_socket);
            break;
        } else if (n == 0) {
            // Client disconnected
            server.removeClient(client_socket);
            std::cout << "Client disconnected on socket " << client_socket << std::endl;
            close(client_socket);
            break;
        }

        buff[n] = '\0';
        std::istringstream stream(buff);
        auto request = Requests(stream, server, client_socket);

        auto RESPONSE = request.Process();
        Server::SendResponse(client_socket, RESPONSE);
    }

}

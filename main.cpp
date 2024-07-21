#include <iostream>
#include <system_error>
#include <thread>
#include "DB/DB.h"
#include "Server/Server.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./Server <port_number>" << std::endl;
        return 1;
    }

    int PORT_NO = std::stoi(argv[1]);
    Server server(PORT_NO);

    DB DataBase;

    if (listen(server.get_SOCKET(), 128) == -1) {
        std::cerr << "Error in listen function: "
                  << std::strerror(errno) << std::endl;
    }

    while (true) {
        socklen_t addrlen = sizeof(server.get_SERVER_ADDR());
        sockaddr_in client_addr{};
        int client_socket = accept(server.get_SOCKET(),
                                   (struct sockaddr*)&client_addr,
                                   &addrlen);
        if (client_socket == -1) {
            std::cerr << "Error in accept function: "
                      << std::strerror(errno) << std::endl;
            continue;
        }
        else{
            std::thread t1(&Server::Receive, client_socket);
            t1.join();
        }
    }

    return 0;
}

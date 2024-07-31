#include "Server.h"
#include "Worker.h"

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

        std::cout << "Server initialized and bound to port " << PORT << std::endl;
    } catch (const std::system_error& error) {
        std::cerr << "Caught system error: " << error.code() << "\n"
                  << error.what() << "\n"
                  << errno << "\n";
    }
}

Server::~Server() {
    close(SOCKET);

    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &client : clients) {
        close(client.second);
    }

    for (auto &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void Server::run() {
    if (listen(SOCKET, 128) == -1) {
        std::cerr << "Error in listen function: " << std::strerror(errno) << std::endl;
        return;
    }

    std::cout << "Server is listening on port " << ntohs(SERVER_ADDR.sin_port) << std::endl;
    std::thread(&Server::acceptConnections, this).detach();

    std::unique_lock<std::mutex> lock(cv_m);
    while (true) {
        cv.wait(lock, [this]{ return !client_sockets.empty(); });
        int client_socket = client_sockets.back();
        client_sockets.pop_back();
        workers.emplace_back(Worker(client_socket, *this));
    }
}

void Server::acceptConnections() {
    while (true) {
        socklen_t addrlen = sizeof(SERVER_ADDR);
        sockaddr_in client_addr{};
        int client_socket = accept(SOCKET, (struct sockaddr*)&client_addr, &addrlen);
        if (client_socket == -1) {
            std::cerr << "Error in accept function: " << std::strerror(errno) << std::endl;
            continue;
        }

        std::lock_guard<std::mutex> lock(cv_m);
        client_sockets.push_back(client_socket);
        cv.notify_one();
    }
}

void Server::RegisterClient(const std::string &phone_number, int client_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients[phone_number] = client_socket;
    std::cout << "Registered client with phone number: " << phone_number << " and socket: " << client_socket << std::endl;
}

bool Server::SendMessage(const std::string &phone_number, const std::string &message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = clients.find(phone_number);

    if (it != clients.end()) {
        int recipient_socket = it->second;
        std::string full_message = message + "\n"; // Add a newline character
        ssize_t sent = send(recipient_socket, full_message.c_str(), full_message.size(), 0);
        if (sent == -1) {
            std::cerr << "Failed to send message to " << phone_number << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "Sent message to " << phone_number << ": " << message << std::endl;
        }
        return true;
    } else {
        std::cerr << "Client with phone number " << phone_number << " not found" << std::endl;
        return false;
    }
}


void Server::SendResponse(int client_socket, const std::string &response) {
    ssize_t sent = send(client_socket, response.c_str(), response.size(), 0);
    if (sent == -1) {
        std::cerr << "Failed to send response to client socket " << client_socket << ": " << strerror(errno) << std::endl;
    } else {
        std::cout << "Sent response to client socket " << client_socket << ": " << response << std::endl;
    }
}

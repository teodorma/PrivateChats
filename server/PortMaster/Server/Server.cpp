#include "Server.h"
#include "Worker.h"
#include "../DB/Encryption/AES.h"

std::unordered_map<std::string, std::pair<int, std::string>> Server::clients;

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
        std::cerr << "Caught system error in Server.cpp at line 22: " << error.code() << "\n"
                  << error.what() << "\n"
                  << errno << "\n";
        throw;
    }
}

void Server::run() {
    if (listen(SOCKET, 128) == -1) {
        std::cerr << "Error in listen function: " << std::strerror(errno) << std::endl;
        return;
    }

    std::cout << "Server is listening on port " << ntohs(SERVER_ADDR.sin_port) << std::endl;
    std::thread(&Server::acceptConnections, this).detach();

    while (true) {
        std::unique_lock<std::mutex> lock(cv_m);
        cv.wait(lock, [this]{ return !client_sockets.empty(); });

        while (!client_sockets.empty()) {
            int client_socket = client_sockets.back();
            client_sockets.pop_back();
            workers.emplace_back([client_socket, this] {
                Worker worker(client_socket, *this);
                worker();
            });
        }
    }
}

void Server::acceptConnections() {
    while (true) {
        socklen_t address_length = sizeof(SERVER_ADDR);
        sockaddr_in client_addr{};
        int client_socket = accept(SOCKET, (struct sockaddr*)&client_addr, &address_length);

        if (client_socket == -1) {
            std::cerr << "Error in accept function: " << std::strerror(errno) << std::endl;
            continue;
        } else {
            std::lock_guard<std::mutex> lock(cv_m);
            client_sockets.push_back(client_socket);
        }
        cv.notify_one();
    }
}

void Server::ConnectClient(const std::string &phone_number, const std::string &aes_key, int client_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients[phone_number] = std::make_pair(client_socket, aes_key);
    std::cout << "Registered client with phone number: " << phone_number << " and socket: " << client_socket << std::endl;
}



bool Server::SendMessage(const std::string &phone_number, const std::string &message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &client : clients) {
        if (client.first == phone_number) {
            int recipient_socket = client.second.first;
            std::string full = AES::encrypt(clients[phone_number].second, message) + "\n";
            ssize_t sent = send(recipient_socket, full.c_str(), full.size(), 0);
            if (sent == -1) {
                std::cerr << "Failed to send message to " << phone_number << ": " << strerror(errno) << std::endl;
            } else {
                std::cout << "Sent message to " << phone_number << ": " << message << std::endl;
            }
            return true;
        }
    }
    std::cerr << "Client with phone number " << phone_number << " not found" << std::endl;
    return false;
}



void Server::removeClient(const int client_sock) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    auto it = std::find_if(clients.begin(), clients.end(),
                           [client_sock](const auto& pair) {
                               return pair.second.first == client_sock;
                           });

    if (it != clients.end()) {
        close(it->second.first);
        std::cout << "Removed client with socket: " << it->second.first << std::endl;
        clients.erase(it);
    } else {
        std::cout << "Client with socket " << client_sock << " not found." << std::endl;
    }
}




void Server::SendResponse(int client_socket, const std::string &response) {
    std::string full_response = Base64_encode(response) + "\n";
    ssize_t sent = send(client_socket, full_response.c_str(), full_response.size(), 0);
    if (sent == -1) {
        std::cerr << "Failed to send response to client socket " << client_socket << ": " << strerror(errno) << std::endl;
    } else {
        std::cout << "Sent response to client socket " << client_socket << ": " << response << std::endl;
    }
    std::cout << std::flush;
}



Server::~Server() {
    close(SOCKET);

    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &client : clients) {
        close(client.second.first);
    }

    for (auto &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}
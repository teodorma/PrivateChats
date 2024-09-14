#include "Worker.h"
#include "Server.h"
#include "../Requests/Requests.h"

Worker::Worker(int client_socket, Server &server) : client_socket(client_socket), server(server) {}

void Worker::operator()() {
    handleClient();
}

void Worker::handleClient() {
    try {
        std::cout << "Started handling client socket " << client_socket << std::endl;
        std::string encrypted = RSA_INSTANCE::getModulus().get_str();
        ssize_t sent = send(client_socket, (encrypted + "\n").c_str(), (encrypted + "\n").size(), 0);

        if (sent == -1) {
            std::cerr << "Failed to send key key_json to " << client_socket << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "Sent key key_json to " << client_socket << ": " << encrypted << std::endl;
        }
        int n;
        char buff[4096] = {0};
        readFromSocket(buff, n);
        if (n <= 0) {
            throw std::runtime_error("Error reading key key_json from client in Worker.cpp at line 25");
        }
        std::string key_response = RSA_INSTANCE::decrypt(buff);
        Json::Value key_json(key_response);

        addKey(key_json);
        Server::SendResponse(client_socket, AES::encrypt(KEY,R"("RESPONSE":"SUCCESS")"));

        while (true) {
            if (readFromSocket(buff, n)) {
                buff[n] = '\0';
                std::string stream(buff);

                auto request = Requests(AES::decrypt(KEY, stream), server, client_socket, KEY);

                auto response = request.Process();
                Server::SendResponse(client_socket, AES::encrypt(KEY, response));
            } else {
                throw std::runtime_error("Error reading key key_json from client in Worker.cpp at line 45");
                break;
            }
        }
    }
    catch(std::runtime_error& e){
        throw;
    }
}



bool Worker::readFromSocket(char (&buff)[4096], int& n){
    n = (int)recv(client_socket, buff, sizeof(buff) - 1, 0);

    if (n < 0) {
        std::cerr << "Error reading from socket: " << strerror(errno) << std::endl;
        close(client_socket);
        return false;
    } else if (n == 0) {
        server.removeClient(client_socket);
        std::cout << "Client disconnected on socket " << client_socket << std::endl;
        close(client_socket);
        return false;
    }
    return true;
}



void Worker::addKey(Json::Value& response){
    KEY = response["KEY"].asString();
}
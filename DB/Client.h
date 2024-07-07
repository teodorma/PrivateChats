//
// Created by maciucateodor on 6/27/24.
//

#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include <string>
#include <queue>

class Client {
    std::string IP;
    std::string PHONE;
    std::queue<std::string> MESSAGES;

    Client() = default;
    explicit Client(std::string IP) : IP(std::move(IP)) {}

    ~Client();

    [[nodiscard]] std::string toString() const {
        return "IP: " + IP + ", Messages Count: " + std::to_string(MESSAGES.size());
    }

};


#endif //SERVER_CLIENT_H

//
// Created by maciucateodor on 6/24/24.
//

#ifndef SERVER_DB_H
#define SERVER_DB_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <iostream>
#include <queue>
#include <utility>
#include <json/json.h>

class DB{
private:
    struct Client {

        std::string IP;
        std::queue<std::string> MESSAGES;

        Client() = default;
        explicit Client(std::string IP) : IP(std::move(IP)) {}

        ~Client();

        [[nodiscard]] std::string toString() const {
            return "IP: " + IP + ", Messages Count: " + std::to_string(MESSAGES.size());
        }
    };

    static std::unordered_map<std::string, Client> DataBase;
    static std::mutex db_mutex;

    static bool Validate(const std::string& PHONE);
    static std::istringstream Update(const std::string& PHONE, const std::string& IP);
    static std::istringstream Delete(const std::string& PHONE);
    static std::istringstream Get(const std::string& PHONE);
    static void AllData();

public:
    ~DB();

    friend class Requests;
};

#endif //SERVER_DB_H

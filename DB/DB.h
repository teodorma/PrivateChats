//
// Created by maciucateodor on 6/24/24.
//

#ifndef SERVER_DB_H
#define SERVER_DB_H
#include <string>
#include <unordered_map>
#include <mutex>
#include <iostream>

class DB {
private:
    static std::unordered_map<std::string, std::string> DataBase;
    static std::mutex db_mutex;

    static bool Validate(const std::string& PHONE);
    static void Update(const std::string& PHONE, const std::string& IP);
    static void Delete(const std::string& PHONE);
    static void AllData();

    friend class Requests;
};


#endif //SERVER_DB_H

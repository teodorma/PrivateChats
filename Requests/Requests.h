//
// Created by maciucateodor on 6/24/24.
//

#ifndef SERVER_REQUESTS_H
#define SERVER_REQUESTS_H
#include <string>
#include <system_error>
#include <iostream>
#include "../DB/DB.h"

class Requests : Encryption{
private:
    Json::Value Request;
public:
    enum TYPES{UPDATE, GET_KEY, DELETE, ALL_DATA, PURGE};
    explicit Requests(std::istringstream & data);
    std::string Process();

    static TYPES getType(const Json::Value& STR);
    static bool isKeyRequest(const std::string &s);

    ~Requests();
};
#endif //SERVER_REQUESTS_H

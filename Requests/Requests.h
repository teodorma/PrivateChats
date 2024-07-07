//
// Created by maciucateodor on 6/24/24.
//

#ifndef SERVER_REQUESTS_H
#define SERVER_REQUESTS_H
#include <string>
#include <system_error>
#include <iostream>
#include "../DB/DB.h"

class Requests : public Json::FastWriter{
private:
    Json::Value Request;

public:
    enum TYPES{UPDATE, GET, DELETE, ALL_DATA};
    explicit Requests(std::istringstream & data);
    Json::Value Process();

    static TYPES getType(const Json::Value& STR);
    ~Requests() override;
};
#endif //SERVER_REQUESTS_H

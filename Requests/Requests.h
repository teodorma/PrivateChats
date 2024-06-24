//
// Created by maciucateodor on 6/24/24.
//

#ifndef SERVER_REQUESTS_H
#define SERVER_REQUESTS_H
#include <string>
#include <json/json.h>
#include <system_error>
#include <iostream>
#include "../DB/DB.h"

class Requests : public Json::FastWriter{
private:
    std::string IP;
    std::string PHONE;
    std::string TYPE;
    Json::Value Request;

public:
    enum TYPES{UPDATE, GET, DELETE, ALL_DATA};
    explicit Requests(std::istringstream & data);
    void Process();

    static TYPES getType(std::string const& STR);

    void setIP();
    void setPHONE();
    void setTYPE();
};
#endif //SERVER_REQUESTS_H

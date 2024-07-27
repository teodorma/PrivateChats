//
// Created by maciucateodor on 7/24/24.
//

#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include "../DB.h"
#include <system_error>
#include <sstream>

class Client {
public:
    static std::istringstream Register(const std::string& PHONE, const std::string& NAME, const std::string& KEY);
    static std::istringstream Message(const std::string &PHONE, const std::string &MESSAGE, const std::string &RECEIVER);

    friend class Requests;
    ~Client();
};


#endif //SERVER_CLIENT_H

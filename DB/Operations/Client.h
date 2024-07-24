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
    static std::istringstream Update(const std::string& PHONE, const std::string& IP);
    static std::istringstream Get_KEY();

    friend class Requests;
};


#endif //SERVER_CLIENT_H

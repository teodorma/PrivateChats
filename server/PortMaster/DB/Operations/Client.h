//
// Created by maciucateodor on 7/24/24.
//

#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

#include "../Database.h"
#include <system_error>
#include <sstream>

class Client {
public:
    static std::istringstream Register(const std::string& PHONE, const std::string& NAME, const std::string& KEY);
    static std::istringstream StoreMessage(const std::string &PHONE, const std::string &JSON);

    friend class Requests;
    ~Client();

    static std::istringstream Get_User_Key(const std::string &PHONE);
    static std::istringstream Get_Messages(const std::string &PHONE, std::vector<std::string> &messages);

    static std::istringstream lookUp(const std::string &Phone);
};


#endif //SERVER_CLIENT_H

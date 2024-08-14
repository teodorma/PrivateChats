//
// Created by maciucateodor on 7/24/24.
//

#ifndef SERVER_ADMIN_H
#define SERVER_ADMIN_H
#include "../DB.h"

class Admin{
public:
    static std::istringstream AllData(const std::string& pass);
    static std::istringstream Delete(const std::string& PHONE, const std::string& pass);
    static std::istringstream Purge(const std::string &pass);

    friend class Requests;

    ~Admin();
};


#endif //SERVER_ADMIN_H

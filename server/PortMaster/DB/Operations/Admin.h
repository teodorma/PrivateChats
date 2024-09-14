#ifndef SERVER_ADMIN_H
#define SERVER_ADMIN_H
#include "../Database.h"

class Admin{
public:
    static std::istringstream AllData(const std::string& pass);
    static std::istringstream Delete(const std::string& PHONE, const std::string& pass);
    static std::istringstream Purge(const std::string &pass);

    friend class Requests;

    ~Admin();
};


#endif //SERVER_ADMIN_H

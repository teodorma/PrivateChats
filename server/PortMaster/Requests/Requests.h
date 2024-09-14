#ifndef SERVER_REQUESTS_H
#define SERVER_REQUESTS_H

#include <string>
#include <system_error>
#include <iostream>
#include "../DB/Database.h"
#include "../DB/Encryption/RSA.h"
#include "../DB/Encryption/AES.h"
#include "../Server/Server.h"
#include <json/json.h>
#include <utility>

class Server;

class Requests {
private:
    Json::Value Request;
    Server& server;
    int client_socket;
    std::string PHONE;
    std::string NAME;
    std::string KEY;
    std::string RECIPIENT_PHONE;
    std::string PASSWORD;
    std::string TYPE;
public:
    Requests(const Json::Value& Request, Server& server, int client_socket,const  std::string& KEY);
    enum TYPES {REGISTER, DELETE, ALL_DATA, PURGE, MESSAGE, GET_USER_KEY, GET_MESSAGES, CONNECT};

    std::string Process();
    TYPES getType();
    ~Requests();
};

#endif // SERVER_REQUESTS_H

#ifndef SERVER_REQUESTS_H
#define SERVER_REQUESTS_H

#include <string>
#include <system_error>
#include <iostream>
#include "../DB/DB.h"
#include "../Server/Server.h"
#include <json/json.h>
#include <utility>

// Forward declare the Server class to avoid cyclic dependency
class Server;

class Requests : Encryption {
public:
    enum TYPES {REGISTER, DELETE, ALL_DATA, PURGE, MESSAGE, GET_USER_KEY, GET_MESSAGES, CONNECT};

    Requests(std::istringstream& data, Server& server, int client_socket);
    std::vector<std::string> Process();


    static TYPES getType(const Json::Value& STR);
    ~Requests();

private:
    static bool isUpdateRequest(const std::string& s);
    Json::Value Request;
    Server& server; // Reference to the Server instance
    int client_socket;
};

#endif // SERVER_REQUESTS_H

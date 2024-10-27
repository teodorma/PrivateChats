#ifndef WORKER_H
#define WORKER_H

#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <thread>
#include <mutex>
#include <json/value.h>
#include "../DB/Encryption/Encryption.h"

class Server;

class Worker : Encryption {
public:
    Worker(int client_socket, Server &server);
    void operator()();

private:
    std::string KEY;
    int client_socket;
    Server &server;

    void handleClient();
    bool readFromSocket(char (&buff)[4096], int &n);
    static std::string parseHttpBody(const std::string &http_message);
    static void sendHttpResponse(int client_socket, const std::string &status, const std::string &body);
    void addKey(Json::Value &response);

    static std::string extractHttpPayload(const std::string &http_message);
};

#endif // WORKER_H

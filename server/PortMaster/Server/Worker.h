#ifndef WORKER_H
#define WORKER_H

#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <map>
#include <json/value.h>

class Server;

class Worker {
public:
    Worker(int client_socket, Server &server);
    void operator()();

private:
    std::string KEY;
    int client_socket;
    Server &server;
    void handleClient();
    bool readFromSocket(char (&buff)[4096], int& n);

    void addKey(Json::Value &response);
};

#endif // WORKER_H

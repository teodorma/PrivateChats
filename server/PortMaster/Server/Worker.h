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

class Server;

class Worker {
public:
    Worker(int client_socket, Server &server);
    void operator()();

private:
    int client_socket;
    Server &server;
    void handleClient();
};

#endif // WORKER_H

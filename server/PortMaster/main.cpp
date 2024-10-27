#include <iostream>
#include "Server/Server.h"
#include "DB/Database.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./Server <port_number>" << std::endl;
        return 1;
    }

    Server server(80);
    RSA_INSTANCE* rsa = RSA_INSTANCE::getInstance();
    std::cout << *rsa;
    Database* Database = Database::getInstance();

    server.run();

    return 0;
}

//
// Created by maciucateodor on 6/24/24.
//

#ifndef SERVER_DB_H
#define SERVER_DB_H
#define DATABASE_NAME "DBSQLITE.db"


#include <string>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <utility>
#include <fstream>
#include <json/json.h>
#include "sqlite3.h"
#include "Encryption/Encryption.h"
#include "Operations/Admin.h"
#include "Operations/Client.h"
#include "../Requests/Requests.h"

class DB : Encryption{
private:
    static mpz_class P;
    static mpz_class Q;
    static mpz_class N;
    static mpz_class D;
    static mpz_class E;
    static int KEY_LENGTH;
    static std::string PASSWORD;
    static std::mutex db_mutex;


public:
    static sqlite3 *DataBase;
    explicit DB();
    DB(const DB &obj) = delete;
    ~DB();

    friend class Admin;
    friend class Client;
    friend class Requests;

    static void createTables();
    static void createDatabase();
    static bool isDatabaseInitiated();
    static void setKeys();
    static void setPassword();

    static bool passCheck(const std::string& password);
    static bool setRSAParameters();
};

#endif //SERVER_DB_H

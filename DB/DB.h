//
// Created by maciucateodor on 6/24/24.
//

#ifndef SERVER_DB_H
#define SERVER_DB_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <iostream>
#include <queue>
#include <utility>
#include <json/json.h>
#include "sqlite3.h"
#include "Encryption.h"

class DB : Encryption{
private:
    static mpz_class P;
    static mpz_class Q;
    static mpz_class N;
    static mpz_class D;
    static mpz_class E;
    static int KEY_LENGTH;
    static sqlite3 *DataBase;
    std::string dbName;
    static std::mutex db_mutex;

    static std::istringstream Update(const std::string& PHONE, const std::string& IP);
    static std::istringstream Delete(const std::string& PHONE);
    static std::istringstream Get_KEY();
    static void AllData();

public:
    explicit DB();
    DB(const DB &obj) = delete;
    ~DB();

    friend class Requests;
    static std::string PASSWORD;
    static void CreateTables();
    static bool isDatabaseInitiated();

    static void setKeys();
    static void setPassword();
};

#endif //SERVER_DB_H

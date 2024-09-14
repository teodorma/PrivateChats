#ifndef SERVER_DATABASE_H
#define SERVER_DATABASE_H
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
#include "Encryption/RSA.h"
#include "Operations/Admin.h"
#include "Operations/Client.h"
#include "../Requests/Requests.h"


class Database : Encryption{
private:
    static std::string PASSWORD;
    static std::mutex db_mutex;
    static Database* instancePtr;
    Database(){
        std::ifstream file(DATABASE_NAME);
        if(!file){
            setPassword();
            createDatabase();
            createTables();
        }
        else{

        }
    };
public:
    static sqlite3 *DataBase;

    Database(const Database &obj) = delete;

    static Database* getInstance();
    friend class Admin;
    friend class Client;
    friend class Requests;

    static void createDatabase();
    static void createTables();
    static void openDatabase();

    void setPassword();
    static bool passCheck(const std::string& password);

    ~Database();
};

#endif //SERVER_DATABASE_H

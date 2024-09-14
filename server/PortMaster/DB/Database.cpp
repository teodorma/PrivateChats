#include "Database.h"

std::mutex Database::db_mutex;
std::string Database::PASSWORD;
Database* Database::instancePtr = nullptr;
sqlite3 *Database::DataBase = nullptr;

Database *Database::getInstance() {
    if (instancePtr == nullptr) {
        std::lock_guard<std::mutex> lock(db_mutex);
        if (instancePtr == nullptr) {
            instancePtr = new Database();
        }
    }
    return instancePtr;
}



void Database::createDatabase() {
    int rc = sqlite3_open(DATABASE_NAME, &DataBase);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(DataBase) << std::endl;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    std::string keyCommand = "PRAGMA key = '" + PASSWORD + "';";
    char* errorMessage = nullptr;
    rc = sqlite3_exec(DataBase, keyCommand.c_str(), nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to set encryption key: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    }
}



void Database::createTables() {
    char* errorMessage = nullptr;

    const char* sqlClient = R"(
        CREATE TABLE IF NOT EXISTS client (
            phone_number TEXT PRIMARY KEY NOT NULL,
            name TEXT NOT NULL,
            key TEXT NOT NULL
        );
    )";

    int rc = sqlite3_exec(DataBase, sqlClient, nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error in client table creation: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Client table created successfully" << std::endl;
    }

    const char* sqlAdmin = R"(
        CREATE TABLE IF NOT EXISTS admin (
            id INTEGER PRIMARY KEY CHECK (id = 0),
            password TEXT NOT NULL
        );
    )";

    rc = sqlite3_exec(DataBase, sqlAdmin, nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error in admin table creation: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Admin table created successfully" << std::endl;
    }

    const char* sqlMessage_Queue = R"(
        CREATE TABLE IF NOT EXISTS message_queue (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            client_phone_number TEXT NOT NULL,
            message TEXT NOT NULL,
            FOREIGN KEY (client_phone_number) REFERENCES client(phone_number)
        );
    )";

    rc = sqlite3_exec(DataBase, sqlMessage_Queue, nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error in message_queue table creation: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Message_queue table created successfully" << std::endl;
    }
}



void Database::setPassword(){
    std::string password1, password2;
    do {
        std::cout << "Enter a strong password: ";
        std::cin >> password1;
        if(!passCheck(password1)){
            std::cout << "Password is too weak\n";
            continue;
        }
        std::cout << "Retype the password: ";
        std::cin >> password2;

        if (password1 != password2) {
            std::cout << "Passwords do not match. Please try again." << std::endl;
        }
    } while (password1 != password2);

    PASSWORD = sha256(password1);
    password1.clear();
    password2.clear();

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}



bool Database::passCheck(const std::string& password){
    if(password.length() < 8) return false;
    bool special;
    bool capt;
    bool number;

    for(auto chr : password){
        if((33 <= chr && chr <= 47) || (58 <= chr && chr <= 64)) number = true;
        if(65 <= chr && chr <= 90) capt = true;
        if(48 <= chr && chr <= 57) special = true;
        if(number && special && capt) return true;
    }
    return false;
}



void Database::openDatabase() {
    int rc = sqlite3_open(DATABASE_NAME, &DataBase);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(DataBase) << std::endl;
        return;
    }

    std::cout << "Opened database successfully" << std::endl;

    std::string keyCommand = "PRAGMA key = '" + PASSWORD + "';";
    char* errorMessage = nullptr;
    rc = sqlite3_exec(DataBase, keyCommand.c_str(), nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to set encryption key: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(DataBase);
        return;
    }

    const char* sql = "PRAGMA user_version;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(DataBase, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare verification statement: " << sqlite3_errmsg(DataBase) << std::endl;
        sqlite3_close(DataBase);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
        std::cerr << "Database verification failed: " << sqlite3_errmsg(DataBase) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(DataBase);
        return;
    } else {
        std::cout << "Database opened successfully with the provided password." << std::endl;
    }

    sqlite3_finalize(stmt);
}



Database::~Database() {
    if (DataBase) {
        sqlite3_close(DataBase);
    }
}

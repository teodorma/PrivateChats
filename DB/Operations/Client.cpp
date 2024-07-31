//
// Created by maciucateodor on 7/24/24.
//

#include "Client.h"

std::istringstream Client::Register(const std::string& PHONE, const std::string& NAME, const std::string& KEY) {
    std::lock_guard<std::mutex> guard(DB::db_mutex);

    std::string sql = "INSERT INTO client (phone_number, name, key) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(DB::DataBase, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }
    sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, NAME.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, KEY.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert data: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }


    sqlite3_finalize(stmt);
    return std::istringstream(R"({"RESPONSE":"REGISTER_SUCCESS", "KEY":")" + DB::N.get_str() + R"("})");
}


std::istringstream Client::Message(const std::string& PHONE, const std::string& MESSAGE, const std::string& RECEIVER){


    return std::istringstream(R"({"RESPONSE":"REGISTER_SUCCESS", "KEY":")" + DB::N.get_str() + R"("})");
}

std::istringstream Client::Get_User_Key(const std::string& PHONE) {
    std::lock_guard<std::mutex> guard(DB::db_mutex);
    const char* sql = "SELECT key FROM client WHERE phone_number = ?;";
    int rc;
    sqlite3_stmt* stmt;

    rc = sqlite3_prepare_v2(DB::DataBase, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE", "KEY":""})");
    }

    rc = sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind value: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE", "KEY":""})");
    }

    rc = sqlite3_step(stmt);
    std::ostringstream oss;

    if (rc == SQLITE_ROW) {
        const unsigned char* keyText = sqlite3_column_text(stmt, 0); // Correct column index for 'key'
        if (keyText) {
            std::string keyStr(reinterpret_cast<const char*>(keyText));
            oss << R"({"RESPONSE":"SUCCESS", "KEY":")" << keyStr << R"("})";
        } else {
            std::cerr << "No key text found: " << sqlite3_errmsg(DB::DataBase) << std::endl;
            sqlite3_finalize(stmt);
            return std::istringstream(R"({"RESPONSE":"NO_DATA", "KEY":""})");
        }
    } else {
        std::cerr << "No data found: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"NO_DATA", "KEY":""})");
    }

    sqlite3_finalize(stmt);
    return std::istringstream(oss.str());
}



Client::~Client() = default;
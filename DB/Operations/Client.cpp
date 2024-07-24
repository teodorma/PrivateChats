//
// Created by maciucateodor on 7/24/24.
//

#include "Client.h"

std::istringstream Client::Update(const std::string& PHONE, const std::string& NAME, const std::string& KEY) {
    std::lock_guard<std::mutex> guard(DB::db_mutex);

    std::string sql = "INSERT INTO client (phone_number, name, key) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(DB::DataBase, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }
    sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, NAME.c_str(), -1, SQLITE_TRANSIENT);  // Using PHONE as the name for now
    sqlite3_bind_text(stmt, 3, KEY.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert data: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    sqlite3_finalize(stmt);
    return std::istringstream(R"({"RESPONSE":"SUCCESS"})");
}


std::istringstream Client::Get_KEY() {
    std::lock_guard<std::mutex> guard(DB::db_mutex);
    std::string response_str(R"({"RESPONSE":"SUCCESS", "KEY":")" + DB::N.get_str() + R"("})");

    return std::istringstream(response_str);
}
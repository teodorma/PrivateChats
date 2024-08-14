//
// Created by maciucateodor on 7/24/24.
//

#include "Admin.h"

std::istringstream Admin::AllData(const std::string& pass) {
    if(pass != DB::PASSWORD){
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }
    std::lock_guard<std::mutex> guard(DB::db_mutex);

    const char* sql = "SELECT phone_number, name, key FROM client;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(DB::DataBase, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");;
    }

    std::cout << "Clients in the database:" << std::endl;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* phone_number = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* key = sqlite3_column_text(stmt, 2);

        std::cout << "Phone Number: " << phone_number << ", Name: " << name << ", KEY: " << key << std::endl;
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to fetch data: " << sqlite3_errmsg(DB::DataBase) << std::endl;
    }

    sqlite3_finalize(stmt);

    return std::istringstream(R"({"RESPONSE":"ALL_DATA_RETRIEVED"})");
}




std::istringstream Admin::Delete(const std::string& PHONE, const std::string& pass) {
    if(pass != DB::PASSWORD){
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }
    std::lock_guard<std::mutex> guard(DB::db_mutex);

    std::string sql = "DELETE FROM client WHERE phone_number = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(DB::DataBase, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to delete data: " << sqlite3_errmsg(DB::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    sqlite3_finalize(stmt);
    return std::istringstream(R"({"RESPONSE":"DELETE_SUCCESS"})");
}



std::istringstream Admin::Purge(const std::string& pass){
    if(pass != DB::PASSWORD){
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }
    remove(DATABASE_NAME);

    sqlite3_close(DB::DataBase);
    return std::istringstream(R"({"RESPONSE":"PURGE_SUCCESS"})");
}


Admin::~Admin() = default;
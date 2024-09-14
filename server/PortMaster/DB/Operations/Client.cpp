#include "Client.h"

std::istringstream Client::Register(const std::string& PHONE, const std::string& NAME, const std::string& KEY) {
    std::lock_guard<std::mutex> guard(Database::db_mutex);

    std::string sql = "INSERT INTO client (phone_number, name, key) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(Database::DataBase, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }
    sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, NAME.c_str(), -1, SQLITE_TRANSIENT);  // Using PHONE as the name for now
    sqlite3_bind_text(stmt, 3, KEY.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert data: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    sqlite3_finalize(stmt);
    return std::istringstream(R"({"RESPONSE":"REGISTER_SUCCESS", "KEY":"("})");
}



std::istringstream Client::StoreMessage(const std::string& PHONE, const std::string& JSON) {
    const char* sql = "INSERT INTO message_queue (client_phone_number, message) VALUES (?, ?);";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(Database::DataBase, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::istringstream(R"({"RESPONSE":"ERROR_PREPARING_STATEMENT"})");
    }
    sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, JSON.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"ERROR_EXECUTING_STATEMENT"})");
    }

    sqlite3_finalize(stmt);

    return std::istringstream(R"({"RESPONSE":"MESSAGE_STORED"})");
}



std::istringstream Client::Get_User_Key(const std::string& PHONE) {
    std::lock_guard<std::mutex> guard(Database::db_mutex);
    const char* sql = "SELECT key FROM client WHERE phone_number = ?;";
    int rc;
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(Database::DataBase, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE", "PHONE":")" + PHONE + R"(", "KEY":""})");
    }

    rc = sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind value: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE", "PHONE":")" + PHONE + R"(", "KEY":""})");
    }

    rc = sqlite3_step(stmt);
    std::ostringstream oss;
    if (rc == SQLITE_ROW) {
        const unsigned char* keyText = sqlite3_column_text(stmt, 0);
        if (keyText) {
            std::string keyStr(reinterpret_cast<const char*>(keyText));
            oss << R"({"RESPONSE":"SUCCESS", "PHONE":")" << PHONE << R"(", "KEY":")" << keyStr << R"("})";
        } else {
            std::cerr << "No key text found: " << sqlite3_errmsg(Database::DataBase) << std::endl;
            sqlite3_finalize(stmt);
            return std::istringstream(R"({"RESPONSE":"NO_DATA", "PHONE":")" + PHONE + R"(", "KEY":""})");
        }
    } else {
        std::cerr << "No data found: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"NO_DATA", "PHONE":")" + PHONE + R"(", "KEY":""})");
    }

    sqlite3_finalize(stmt);
    return std::istringstream(oss.str());
}



std::istringstream Client::Get_Messages(const std::string& PHONE, std::vector<std::string>& messages) {
    const char* sql = "SELECT message FROM message_queue WHERE client_phone_number = ?";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(Database::DataBase, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    rc = sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind value: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    int count = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text) {
            messages.emplace_back(reinterpret_cast<const char*>(text));
            ++count;
        }
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    sqlite3_finalize(stmt);

    if (count == 0) {
        std::cerr << "No messages were found for phone number: " << PHONE << std::endl;
    }

    return std::istringstream(R"({"RESPONSE":"SUCCESS"})");
}



std::istringstream Client::lookUp(const std::string& PHONE) {
    const char* sql = "SELECT EXISTS(SELECT 1 FROM client WHERE phone_number = ?);";
    int rc;
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(Database::DataBase, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    rc = sqlite3_bind_text(stmt, 1, PHONE.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind value: " << sqlite3_errmsg(Database::DataBase) << std::endl;
        sqlite3_finalize(stmt);
        return std::istringstream(R"({"RESPONSE":"FAILURE"})");
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text == nullptr) {
            return std::istringstream(R"({"RESPONSE":"FAILURE"})");
        }
    }

    return std::istringstream(R"({"RESPONSE":"SUCCESS"})");
}



Client::~Client() = default;
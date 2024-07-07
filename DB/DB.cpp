#include "DB.h"

std::unordered_map<std::string, DB::Client> DB::DataBase;
std::mutex DB::db_mutex;

bool DB::Validate(const std::string& PHONE) {
    std::lock_guard<std::mutex> guard(db_mutex);
    return DataBase.find(PHONE) != DataBase.end();
}

std::istringstream DB::Update(const std::string& PHONE, const std::string& IP) {
    std::lock_guard<std::mutex> guard(db_mutex);
    auto client = Client(IP);
    DataBase[PHONE] = client;

    std::istringstream RESPONSE(R"({"RESPONSE":"SUCCESS"})");

    return RESPONSE;
}

std::istringstream DB::Delete(const std::string& PHONE) {
    std::lock_guard<std::mutex> guard(db_mutex);
    DataBase.erase(PHONE);
    std::istringstream RESPONSE(R"({"RESPONSE":"SUCCESS"})");

    return RESPONSE;
}

std::istringstream DB::Get(const std::string& PHONE) {
    std::lock_guard<std::mutex> guard(db_mutex);
    if (DataBase.find(PHONE) != DataBase.end()) {
        Client& client = DataBase[PHONE];
        std::string token;
        while (!client.MESSAGES.empty()) {
            token += client.MESSAGES.front() + "\n";
            client.MESSAGES.pop();
        }
        std::istringstream messages(token);
        return messages;
    }
    std::istringstream messages("No messages found for this phone number.");
    return messages;
}

void DB::AllData() {
    std::lock_guard<std::mutex> guard(db_mutex);
    for (const auto& entry : DataBase) {
        std::cout << "Phone: " << entry.first << ", " << entry.second.toString() << std::endl;
    }
}

DB::Client::~Client() = default;
DB::~DB() = default;

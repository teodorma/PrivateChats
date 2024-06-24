//
// Created by maciucateodor on 6/24/24.
//

#include "DB.h"

std::unordered_map<std::string, std::string> DB::DataBase;
std::mutex DB::db_mutex;

bool DB::Validate(const std::string& PHONE) {
    std::lock_guard<std::mutex> guard(db_mutex);
    return DataBase.find(PHONE) != DataBase.end();
}

void DB::Update(const std::string& PHONE, const std::string& IP) {
    std::lock_guard<std::mutex> guard(db_mutex);
    DataBase[PHONE] = IP;
}

void DB::Delete(const std::string& PHONE) {
    std::lock_guard<std::mutex> guard(db_mutex);
    DataBase.erase(PHONE);
}

void DB::AllData() {
    for(const auto& i : DataBase){
        std::cout << i.first << " "
                  << i.second << std::endl;
    }
}

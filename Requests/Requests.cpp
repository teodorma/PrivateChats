#include "Requests.h"

Requests::Requests(std::istringstream& data) {
    data >> Request;
    setIP();
    setPHONE();
    setTYPE();
}

void Requests::Process() {
    std::cout << Request << std::endl;
    switch (getType(TYPE)) {
        case UPDATE: {
            std::cout << "1";
            DB::Update(PHONE, IP);
            break;
        }
        case GET: {
            // Handle GET request
            break;
        }
        case DELETE: {
            DB::Delete(PHONE);
            break;
        }
        case ALL_DATA: {
            std::cout << "2";
            DB::AllData();
            break;
        }
        default: {
            std::cerr << "Invalid request type";
            return;
        }
    }
}

Requests::TYPES Requests::getType(const std::string& STR) {
    if (STR == "UPDATE") return UPDATE;
    if (STR == "GET") return GET;
    if (STR == "DELETE") return DELETE;
    if (STR == "ALL_DATA") return ALL_DATA;

    throw std::invalid_argument("Invalid request type");
}

void Requests::setIP() {
    IP = Request["IP"].asString();
}

void Requests::setPHONE() {
    PHONE = Request["PHONE"].asString();
}

void Requests::setTYPE() {
    TYPE = Request["TYPE"].asString();
}

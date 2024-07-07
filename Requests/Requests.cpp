#include "Requests.h"

Requests::Requests(std::istringstream& data) {
    data >> Request;
}

Json::Value Requests::Process() {
    std::cout << Request << std::endl;
    std::istringstream MES;
    Json::Value RESPONSE;

    switch (getType(Request)) {
        case UPDATE: {
            DB::Update(Request["PHONE"].asString(), Request["IP"].asString()) >> RESPONSE;
            break;
        }
        case GET: {
            DB::Get(Request["PHONE"].asString()) >> RESPONSE;
            break;
        }
        case DELETE: {
            DB::Delete(Request["PHONE"].asString()) >> RESPONSE;
            break;
        }
        case ALL_DATA: {
            DB::AllData();
            break;
        }
        default: {
            RESPONSE = "Invalid request type";
            return "";
        }
    }
    return RESPONSE;
}

Requests::TYPES Requests::getType(const Json::Value& STR) {
    std::string types = STR["TYPE"].asString();

    if (types == "UPDATE") return UPDATE;
    if (types == "GET") return GET;
    if (types == "DELETE") return DELETE;
    if (types == "ALL_DATA") return ALL_DATA;

    throw std::invalid_argument("Invalid request type");
}

Requests::~Requests() = default;
#include "Requests.h"

Requests::Requests(std::istringstream& data, Server& server, int client_socket) : server(server), client_socket(client_socket) {
    std::cout << std::endl;
    std::string dataStr = data.str();
    std::cout << "Received data: " << dataStr << std::endl;
    if (isUpdateRequest(dataStr)) {
        std::istringstream ss(dataStr);
        ss >> Request;
    } else {
        //std::string decryptedData = decrypt(dataStr, DB::D, DB::N);
        std::cout << "Decrypted data: " << dataStr << std::endl;

        std::istringstream ss(dataStr);
        ss >> Request;
    }
}

bool Requests::isUpdateRequest(const std::string& s) {
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;
    std::istringstream ss(s);
    if (Json::parseFromStream(readerBuilder, ss, &root, &errs)) {
        return root.isObject() && root.isMember("TYPE") && root["TYPE"].asString() == "REGISTER";
    } else {
        return false;
    }
}


std::string Requests::Process() {
    Json::Value JSON;
    switch (getType(Request)) {
        case REGISTER: {
            std::string phone_number = getPhoneNumber();
            server.RegisterClient(phone_number, client_socket);
            Client::Register(Request["PHONE"].asString(),
                             Request["NAME"].asString(),
                             Request["KEY"].asString()) >> JSON;
            break;
        }
        case DELETE: {
            Admin::Delete(Request["PHONE"].asString(),
                          Request["PASSWORD"].asString()) >> JSON;
            break;
        }
        case ALL_DATA: {
            Admin::AllData(Request["PASSWORD"].asString()) >> JSON;
            break;
        }
        case PURGE: {
            Admin::Purge(Request["PASSWORD"].asString()) >> JSON;
            break;
        }
        case MESSAGE: {
            std::string recipient_phone = getRecipientPhoneNumber();
            std::string message = getMessage();
            if(!server.SendMessage(recipient_phone, message)){
                Client::StoreMessage(Request["PHONE"].asString(), Request.asString()) >> JSON;
            }
            else{
                JSON["RESPONSE"] = "MESSAGE_SENT";
            }
            break;
        }
        default: {
            std::istringstream(R"({"RESPONSE":"INVALID_REQUEST"})") >> JSON;
            break;
        }
    }
    Json::FastWriter WRITER;
    auto RESPONSE = WRITER.write(JSON);
    return RESPONSE;
}

Requests::TYPES Requests::getType(const Json::Value& STR) {
    std::string types = STR["TYPE"].asString();

    if (types == "REGISTER") return REGISTER;
    if (types == "DELETE") return DELETE;
    if (types == "ALL_DATA") return ALL_DATA;
    if (types == "PURGE") return PURGE;
    if (types == "MESSAGE") return MESSAGE;

    throw std::invalid_argument("Invalid request type");
}

std::string Requests::getPhoneNumber() const {
    return Request["PHONE"].asString();
}

std::string Requests::getRecipientPhoneNumber() const {
    return Request["RECIPIENT_PHONE"].asString();
}

std::string Requests::getMessage() const {
    return Request["MESSAGE"].asString();
}

Requests::~Requests() = default;

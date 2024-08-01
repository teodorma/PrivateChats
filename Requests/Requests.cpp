#include "Requests.h"

Requests::Requests(std::istringstream& data, Server& server, int client_socket) : server(server), client_socket(client_socket) {
    std::cout << std::endl;
    std::string dataStr = data.str();
    std::cout << "Received data: " << dataStr;
    if (isUpdateRequest(dataStr)) {
        std::istringstream ss(dataStr);
        ss >> Request;
    } else {
        std::string decryptedData = decrypt(dataStr, DB::D, DB::N);
        std::cout << "Decrypted data: " << decryptedData << std::endl;

        std::istringstream ss(decryptedData);
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
    Json::FastWriter WRITER;
    switch (getType(Request)) {
        case REGISTER: {
            server.RegisterClient(Request["PHONE"].asString(), client_socket);
            Client::Register(Request["PHONE"].asString(),
                             Request["NAME"].asString(),
                             Request["KEY"].asString()) >> JSON;
            break;
        }
        case CONNECT: {
            Client::lookUp(Request["PHONE"].asString()) >> JSON;
            if (JSON["RESPONSE"].asString() == "SUCCESS") {
                server.RegisterClient(Request["PHONE"].asString(), client_socket);
            }
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
        case GET_USER_KEY: {
            Client::Get_User_Key(Request["PHONE"].asString()) >> JSON;
            break;
        }
        case GET_MESSAGES: {
            std::vector<std::string> messages;
            Client::Get_Messages(Request["PHONE"].asString(), messages) >> JSON;
            for(const auto& chunk : messages){
                server.SendMessage(Request["PHONE"].asString(), chunk);
            }
            break;
        }
        case MESSAGE: {
            auto request = WRITER.write(Request);
            if(!server.SendMessage(Request["RECIPIENT_PHONE"].asString(), request)){
                Client::StoreMessage(Request["PHONE"].asString(), request) >> JSON;
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
    if (types == "GET_USER_KEY") return GET_USER_KEY;
    if (types == "GET_MESSAGES") return GET_MESSAGES;
    if (types == "CONNECT") return CONNECT;

    throw std::invalid_argument("Invalid request type");
}

Requests::~Requests() = default;

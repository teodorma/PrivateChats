#include "Requests.h"
Requests::Requests(const Json::Value &Request, Server &server, int client_socket, const std::string& KEY)
: Request(Request), server(server), client_socket(client_socket), KEY(KEY) {
    std::cout << "Decrypted data: " << Request.asString() << std::endl;

    RECIPIENT_PHONE = Request["RECIPIENT_PHONE"].asString();
    PHONE = Request["PHONE"].asString();
    NAME = Request["NAME"].asString();
    PASSWORD = Request["PASSWORD"].asString();
    TYPE = Request["TYPE"].asString();
}



std::string Requests::Process() {
    Json::Value JSON;
    Json::FastWriter WRITER;
    switch (getType()) {
        case REGISTER: {
            server.ConnectClient(Request["PHONE"].asString(), KEY, client_socket);
            Client::Register(Request["PHONE"].asString(),
                             Request["NAME"].asString(),
                             Request["KEY"].asString()) >> JSON;
            break;
        }
        case CONNECT: {
            Client::lookUp(Request["PHONE"].asString()) >> JSON;
            if (JSON["RESPONSE"].asString() == "SUCCESS"){
                server.ConnectClient(Request["PHONE"].asString(), KEY, client_socket);
            }
            break;
        }
        case DELETE: {
            Admin::Delete(PHONE, PASSWORD) >> JSON;
            break;
        }
        case ALL_DATA: {
            Admin::AllData(PASSWORD) >> JSON;
            break;
        }
        case PURGE: {
            Admin::Purge(PASSWORD) >> JSON;
            break;
        }
        case GET_MESSAGES: {
            std::vector<std::string> messages;
            Client::Get_Messages(RECIPIENT_PHONE, messages) >> JSON;

            for (const auto &message : messages) {
                server.SendMessage(RECIPIENT_PHONE, message);
            }
            break;
        }

        case MESSAGE: {
            auto request = AES::encrypt(KEY , WRITER.write(Request));
            if (!server.SendMessage(RECIPIENT_PHONE, request)) {
                Client::StoreMessage(RECIPIENT_PHONE, request) >> JSON;
            } else {
                JSON["RESPONSE"] = "MESSAGE_SENT";
            }
            break;
        }
        default: {
            std::istringstream(R"({"RESPONSE":"INVALID_REQUEST"})") >> JSON;
            break;
        }
    }

    std::string responseStr = WRITER.write(JSON);
    return responseStr;
}



Requests::TYPES Requests::getType() {
    std::string types = TYPE;

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

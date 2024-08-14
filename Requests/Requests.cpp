#include "Requests.h"
#include <zlib.h>

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

std::vector<std::string> Requests::Process() {
    Json::Value JSON;
    Json::FastWriter WRITER;
    switch (getType(Request)) {
        case REGISTER: {
            server.ConnectClient(Request["PHONE"].asString(), client_socket);
            Client::Register(Request["PHONE"].asString(),
                             Request["NAME"].asString(),
                             Request["KEY"].asString()) >> JSON;
            break;
        }
        case CONNECT: {
            Client::lookUp(Request["PHONE"].asString()) >> JSON;
            if (JSON["RESPONSE"].asString() == "SUCCESS"){
                server.ConnectClient(Request["PHONE"].asString(), client_socket);
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
            Client::Get_User_Key(Request["RECIPIENT_PHONE"].asString()) >> JSON;
            break;
        }
        case GET_MESSAGES: {
            std::vector<std::string> messages;
            Client::Get_Messages(Request["RECIPIENT_PHONE"].asString(), messages) >> JSON;

            if(!messages.empty()){
                Json::Value KEY_JSON;
                Client::Get_User_Key(Request["RECIPIENT_PHONE"].asString()) >> KEY_JSON;
                mpz_class key;
                key.set_str(KEY_JSON["KEY"].asString(), 16);

                std::cout << key << std::endl;
                for (const auto &chunk : messages) {
                    server.SendMessage(Request["RECIPIENT_PHONE"].asString(), encrypt(chunk, DB::E, key));
                }
            }
            break;
        }
        case MESSAGE: {
            Json::Value KEY_JSON;
            Client::Get_User_Key(Request["RECIPIENT_PHONE"].asString()) >> KEY_JSON;
            mpz_class key;
            key.set_str(KEY_JSON["KEY"].asString(), 16);
            auto request = WRITER.write(Request);
            if (!server.SendMessage(Request["RECIPIENT_PHONE"].asString(), encrypt(request, DB::E, key))) {
                Client::StoreMessage(Request["RECIPIENT_PHONE"].asString(), request) >> JSON;
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
    Json::Value KEY;
    Client::Get_User_Key(Request["PHONE"].asString()) >> KEY;
    mpz_class ClientPublicKey;
    ClientPublicKey.set_str(KEY["KEY"].asString(), 16);

    auto split_string_into_chunks = [](const std::string& str, size_t chunk_size) {
        std::vector<std::string> chunks;
        for (size_t i = 0; i < str.length(); i += chunk_size) {
            chunks.push_back(str.substr(i, chunk_size));
        }
        return chunks;
    };

    std::string responseStr = WRITER.write(JSON);
    size_t chunk_size = 190;
    std::vector<std::string> chunks = split_string_into_chunks(responseStr, chunk_size);

    std::vector<std::string> RESPONSE;

    for (size_t i = 0; i < chunks.size(); ++i) {
        Json::Value chunkJson;
        chunkJson["CHUNK_NUMBER"] = static_cast<int>(i);
        chunkJson["NUMBER_OF_CHUNKS"] = chunks.size();
        chunkJson["CHUNK"] = chunks[i];
        std::string chunkStr = WRITER.write(chunkJson);
        std::string encryptedChunk = encrypt(chunkStr, DB::E, ClientPublicKey);
        RESPONSE.push_back(encryptedChunk);
    }

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

#include "Requests.h"

Requests::Requests(std::istringstream& data) {
    std::cout << std::endl;
    std::string dataStr = data.str();
    std::cout << "Received data: " << dataStr << std::endl;
    std::string decodedData = DB::Base64_decode(dataStr);
    std::cout << "Decoded data: " << DB::Base64_decode(dataStr) << std::endl;
    if(isKeyRequest(decodedData)){
        std::istringstream ss(decodedData);
        ss >> Request;
    }
    else {
        std::string decryptedData = DB::decrypt(decodedData, DB::D, DB::N);
        std::cout << "Decrypted data: " << decryptedData << std::endl;

        std::istringstream ss(decryptedData);
        ss >> Request;
    }
}

bool Requests::isKeyRequest(const std::string& s) {
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;
    std::istringstream ss(s);
    if (Json::parseFromStream(readerBuilder, ss, &root, &errs)) {
        return root.isObject() && root.isMember("TYPE") && root["TYPE"].asString() == "GET_KEY";
    } else {
        return false;
    }
}


std::string Requests::Process() {
    Json::Value JSON;
    switch (getType(Request)) {
        case UPDATE: {
            DB::Update(Request["PHONE"].asString(), Request["IP"].asString()) >> JSON;
            break;
        }
        case GET_KEY: {
            DB::Get_KEY() >> JSON;
            break;
        }
        case DELETE: {
            DB::Delete(Request["PHONE"].asString()) >> JSON;
            break;
        }
        case ALL_DATA: {
            DB::AllData();
            break;
        }
        default: {
            std::cout << "Invalid request type" << std::endl;
            break;
        }
    }
    Json::FastWriter WRITER;
    auto RESPONSE = WRITER.write(JSON);
    return RESPONSE;
}

Requests::TYPES Requests::getType(const Json::Value& STR) {
    std::string types = STR["TYPE"].asString();

    if (types == "UPDATE") return UPDATE;
    if (types == "GET_KEY") return GET_KEY;
    if (types == "DELETE") return DELETE;
    if (types == "ALL_DATA") return ALL_DATA;

    throw std::invalid_argument("Invalid request type");
}

Requests::~Requests() = default;


#include "Worker.h"
#include "Server.h"
#include "../Requests/Requests.h"

Worker::Worker(int client_socket, Server &server) : client_socket(client_socket), server(server) {}

void Worker::operator()() {
    handleClient();
}

void Worker::handleClient() {
    try {
        std::cout << "Started handling client socket " << client_socket << std::endl;

        std::string modulus = RSA_INSTANCE::getModulus().get_str();
        std::string encrypted = R"({"KEY": ")" + modulus + "\"}";
        sendHttpResponse(client_socket, "200 OK", encrypted);

        int n;
        char buff[4096];
        if (!readFromSocket(buff, n)) {
            std::cerr << "Error reading initial key from client in Worker.cpp" << std::endl;
            return;
        }

        std::string key_response(buff, n);
        std::string payload = extractHttpPayload(key_response);

        payload = RSA_INSTANCE::decrypt(payload);
        Json::Value key_json;
        std::stringstream(payload) >> key_json;

        if (!key_json.isMember("KEY")) {
            std::cerr << "Invalid JSON: Missing 'KEY' field" << std::endl;
            close(client_socket);
            return;
        }

        addKey(key_json);
        sendHttpResponse(client_socket, "200 OK", AES::encrypt(KEY, R"({"RESPONSE":"SUCCESS"})"));

        while (true) {
            if (readFromSocket(buff, n)) {
                std::string http_message(buff, n);
                std::string body = extractHttpPayload(http_message);

                if (body.empty()) {
                    std::cerr << "No HTTP body detected" << std::endl;
                    continue;
                }

                auto request = Requests(AES::decrypt(KEY, body), server, client_socket, KEY);
                auto response = request.Process();

                sendHttpResponse(client_socket, "200 OK", AES::encrypt(KEY, response));
            } else {
                std::cerr << "Client disconnected in Worker.cpp" << std::endl;
                break;
            }
        }
    } catch (std::runtime_error &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }
}

bool Worker::readFromSocket(char (&buff)[4096], int &n) {
    n = recv(client_socket, buff, sizeof(buff) - 1, 0);

    if (n < 0) {
        std::cerr << "Error reading from socket: " << strerror(errno) << std::endl;
        close(client_socket);
        return false;
    } else if (n == 0) {
        server.removeClient(client_socket);
        std::cout << "Client disconnected on socket " << client_socket << std::endl;
        close(client_socket);
        return false;
    }
    return true;
}

std::string Worker::extractHttpPayload(const std::string &http_message) {
    std::size_t header_end = http_message.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return "";
    }

    return http_message.substr(header_end + 4);
}

void Worker::sendHttpResponse(int client_socket, const std::string &status, const std::string &body) {
    std::string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    response += "Content-Type: application/json\r\n";
    response += "\r\n";
    response += body;

    ssize_t sent = send(client_socket, response.c_str(), response.size(), 0);
    if (sent == -1) {
        std::cerr << "Failed to send HTTP response: " << strerror(errno) << std::endl;
    }
}

void Worker::addKey(Json::Value &response) {
    std::string key_base64 = response["KEY"].asString();
    KEY = Base64_decode(key_base64);
}

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <gmpxx.h>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>

class Encryption {
public:
    static std::string Base64_encode(const std::string& num);
    static std::string Base64_decode(const std::string& b64string);
    ~Encryption();

    std::string sha256(const std::string &str);
};

#endif // ENCRYPTION_H

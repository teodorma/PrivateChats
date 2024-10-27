#include "Encryption.h"


std::string Encryption::sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.length());
    SHA256_Final(hash, &sha256);

    std::string hashStr;
    char hex[3];
    for(unsigned char i : hash) {
        sprintf(hex, "%02x", i);
        hashStr += hex;
    }

    return hashStr;
}



std::string Encryption::Base64_encode(const std::string& input) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input.data(), input.size());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string output(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return output;
}


std::string Encryption::Base64_decode(const std::string& input) {
    BIO *bio, *b64;
    int decodeLen = input.size();
    std::vector<unsigned char> buffer(decodeLen);

    bio = BIO_new_mem_buf(input.data(), -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    decodeLen = BIO_read(bio, buffer.data(), decodeLen);
    BIO_free_all(bio);

    buffer.resize(decodeLen);
    std::string output(buffer.begin(), buffer.end());
    return output;
}



Encryption::~Encryption() = default;

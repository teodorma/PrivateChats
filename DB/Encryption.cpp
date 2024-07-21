#include "Encryption.h"
#include <random>
#include <sstream>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>


const std::vector<int> first_primes = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37,
        41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83,
        89, 97, 101, 103, 107, 109, 113, 127, 131,
        137, 139, 149, 151, 157, 163, 167, 173, 179,
        181, 191, 193, 197, 199, 211, 223, 227, 229,
        233, 239, 241, 251, 257, 263, 269, 271, 277,
        281, 283, 293, 307, 311, 313, 317, 331, 337,
        347, 349, 353, 359, 367, 373, 379, 383, 389,
        397, 401, 409, 419, 421, 431, 433, 439, 443,
        449, 457, 461, 463, 467, 479, 487, 491, 499,
        503, 509, 521, 523, 541, 547, 557, 563, 569,
        571, 577, 587, 593, 599, 601, 607, 613, 617,
        619, 631, 641, 643, 647, 653, 659, 661, 673,
        677, 683, 691, 701, 709, 719, 727, 733, 739,
        743, 751, 757, 761, 769, 773, 787, 797, 809,
        811, 821, 823, 827, 829, 839, 853, 857, 859,
        863, 877, 881, 883, 887, 907, 911, 919, 929,
        937, 941, 947, 953, 967, 971, 977, 983, 991,
        997, 1009, 1013, 1019, 1021, 1031, 1033, 1039,
        1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093,
        1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153,
        1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217,
        1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279,
        1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319,
        1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409,
        1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453
};

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

mpz_class Encryption::genKEY(const int len) {
    while (true) {
        mpz_class candidate = getLowLevelPrime(len);
        if (MillerRabinTest(candidate))
            return candidate;
    }
}

mpz_class Encryption::getRandomBigInt(int bits) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distr;

    mpz_class random_num;
    for (int i = 0; i < bits / 64; ++i) {
        random_num <<= 64;
        random_num += distr(gen);
    }

    random_num |= (mpz_class(1) << (bits - 1));
    random_num |= 1;
    return random_num;
}

mpz_class Encryption::getLowLevelPrime(const int len) {
    while (true) {
        mpz_class candidate = getRandomBigInt(len);
        bool is_prime = true;
        for (int prime : first_primes) {
            if (candidate % prime == 0 && candidate != prime) {
                is_prime = false;
                break;
            }
        }
        if (is_prime)
            return candidate;
    }
}

bool Encryption::trialComposite(const mpz_class& a, const mpz_class& evenC, const mpz_class& to_test, int max_div_2) {
    mpz_class x;
    mpz_powm(x.get_mpz_t(), a.get_mpz_t(), evenC.get_mpz_t(), to_test.get_mpz_t());
    if (x == 1)
        return false;

    for (int i = 0; i < max_div_2; ++i) {
        mpz_class temp;
        mpz_ui_pow_ui(temp.get_mpz_t(), 2, i);
        mpz_mul(temp.get_mpz_t(), temp.get_mpz_t(), evenC.get_mpz_t());
        mpz_powm(x.get_mpz_t(), a.get_mpz_t(), temp.get_mpz_t(), to_test.get_mpz_t());
        if (x == to_test - 1)
            return false;
    }
    return true;
}

bool Encryption::MillerRabinTest(const mpz_class& to_test) {
    constexpr int accuracy = 20;

    int max_div_2 = 0;
    mpz_class evenC = to_test - 1;
    while (mpz_divisible_2exp_p(evenC.get_mpz_t(), 1)) {
        evenC >>= 1;
        max_div_2++;
    }

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distr(2, mpz_get_ui(to_test.get_mpz_t()) - 2);

    for (int i = 0; i < accuracy; ++i) {
        mpz_class a = distr(gen);

        if (trialComposite(a, evenC, to_test, max_div_2))
            return false;
    }
    return true;
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

std::string Encryption::encrypt(const std::string& message, const mpz_class& e, const mpz_class& n) {
    std::stringstream ss;
    for (char c : message) {
        mpz_class m(static_cast<int>(c));
        mpz_class encrypted;
        mpz_powm(encrypted.get_mpz_t(), m.get_mpz_t(), e.get_mpz_t(), n.get_mpz_t());
        ss << encrypted.get_str() << " ";
    }
    return ss.str();
}

std::string Encryption::decrypt(const std::string& ciphertext, const mpz_class& d, const mpz_class& n) {
    mpz_class encrypted(ciphertext);

    mpz_class decrypted;
    mpz_powm(decrypted.get_mpz_t(), encrypted.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());

    size_t count = 0;
    auto* decryptedBytes = (unsigned char*) mpz_export(nullptr, &count, 1, sizeof(unsigned char), 0, 0, decrypted.get_mpz_t());

    std::string plaintext(reinterpret_cast<char*>(decryptedBytes), count);

    free(decryptedBytes);

    return plaintext;
}

mpz_class Encryption::calculatePrivateExponent(const mpz_class& p, const mpz_class& q, const mpz_class& e) {
    mpz_class phi = (p - 1) * (q - 1);

    mpz_class d;
    if (mpz_invert(d.get_mpz_t(), e.get_mpz_t(), phi.get_mpz_t()) == 0) {
        throw std::runtime_error("Error: e does not have an inverse modulo φ(n)");
    }

    return d;
}

Encryption::~Encryption() = default;

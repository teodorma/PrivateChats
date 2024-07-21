#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <gmpxx.h>
#include <vector>
#include <string>

class Encryption {
public:
    static mpz_class genKEY(int len);
    static mpz_class getRandomBigInt(int bits);
    static mpz_class getLowLevelPrime(int len);
    static bool trialComposite(const mpz_class& a, const mpz_class& evenC, const mpz_class& to_test, int max_div_2);
    static bool MillerRabinTest(const mpz_class& to_test);
    static std::string Base64_encode(const std::string& num);
    static std::string Base64_decode(const std::string& b64string);
    static std::string encrypt(const std::string& message, const mpz_class& e, const mpz_class& n);
    static std::string decrypt(const std::string& ciphertext, const mpz_class& d, const mpz_class& n);
    static mpz_class calculatePrivateExponent(const mpz_class& p, const mpz_class& q, const mpz_class& e);
    ~Encryption();

    static std::string sha256(const std::string &str);
};

#endif // ENCRYPTION_H

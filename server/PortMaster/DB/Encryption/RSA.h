#ifndef SERVER_RSA_H
#define SERVER_RSA_H


#include <gmpxx.h>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <iostream>
#include <mutex>
#include <utility>
#include <thread>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>

class RSA_INSTANCE{
private:
    static mpz_class PRIME_NUMBER_1;
    static mpz_class PRIME_NUMBER_2;
    static mpz_class MODULUS;
    static mpz_class PRIVATE_EXPONENT;
    static mpz_class PUBLIC_EXPONENT;
    static int KEY_LENGTH;
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
            937, 941, 947, 953, 967, 971, 977, 983, 991
    };
    RSA_INSTANCE(){
        KEY_LENGTH = 2048/2;
        PRIME_NUMBER_1 = generateKey();
        PRIME_NUMBER_2 = generateKey();
        PUBLIC_EXPONENT = 65537;
        MODULUS = PRIME_NUMBER_1 * PRIME_NUMBER_2;
        PRIVATE_EXPONENT = calculatePrivateExponent();
    }
    static RSA_INSTANCE* instancePtr;
    static std::mutex mtx;

    mpz_class generateKey();
    mpz_class getLowLevelPrime();
    [[nodiscard]] static mpz_class getRandomBigInt() ;
    static bool trialComposite(const mpz_class &a, const mpz_class &evenC, const mpz_class &to_test, int max_div_2);
    static bool MillerRabinTest(const mpz_class &to_test);
    static mpz_class calculatePrivateExponent();
public:
    static mpz_class getPrivateExponent(){ return PRIVATE_EXPONENT; }
    static mpz_class getFirstPrime(){ return PRIME_NUMBER_1; }
    static mpz_class getSecondPrime(){ return PRIME_NUMBER_2; }
    static mpz_class getModulus(){ return MODULUS; }

    static std::string encrypt(const std::string &message);
    static std::string decrypt(const std::string &ciphertext);
    static RSA_INSTANCE* getInstance();
    RSA_INSTANCE(const RSA_INSTANCE& obj) = delete;
    friend std::ostream operator << (std::ostream& out, RSA_INSTANCE& rsa);

    ~RSA_INSTANCE();
};

#endif //SERVER_RSA_H

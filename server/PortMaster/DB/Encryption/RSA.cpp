#include "RSA.h"

RSA_INSTANCE* RSA_INSTANCE::instancePtr = nullptr;
std::mutex RSA_INSTANCE::mtx;
mpz_class RSA_INSTANCE::PRIME_NUMBER_1;
mpz_class RSA_INSTANCE::PRIME_NUMBER_2;
mpz_class RSA_INSTANCE::MODULUS;
mpz_class RSA_INSTANCE::PRIVATE_EXPONENT;
mpz_class RSA_INSTANCE::PUBLIC_EXPONENT;
int RSA_INSTANCE::KEY_LENGTH = 2048/2;


RSA_INSTANCE *RSA_INSTANCE::getInstance() {
    if (instancePtr == nullptr) {
        std::lock_guard<std::mutex> lock(mtx);
        instancePtr = new RSA_INSTANCE();
    }
    return instancePtr;
}



mpz_class RSA_INSTANCE::generateKey() {
    while (true) {
        mpz_class candidate = getLowLevelPrime();
        if (MillerRabinTest(candidate))
            return candidate;
    }
}



mpz_class RSA_INSTANCE::getLowLevelPrime() {
    while (true) {
        mpz_class candidate = getRandomBigInt();
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



mpz_class RSA_INSTANCE::getRandomBigInt() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distribution;

    mpz_class random_num;
    for (int i = 0; i < KEY_LENGTH / 64; ++i) {
        random_num <<= 64;
        random_num += distribution(gen);
    }

    random_num |= (mpz_class(1) << (KEY_LENGTH - 1));
    random_num |= 1;
    return random_num;
}



bool RSA_INSTANCE::MillerRabinTest(const mpz_class& to_test) {
    constexpr int accuracy = 20;

    int max_div_2 = 0;
    mpz_class evenC = to_test - 1;
    while (mpz_divisible_2exp_p(evenC.get_mpz_t(), 1)) {
        evenC >>= 1;
        max_div_2++;
    }

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distribution(2, mpz_get_ui(to_test.get_mpz_t()) - 2);

    for (int i = 0; i < accuracy; ++i) {
        mpz_class a = distribution(gen);

        if (trialComposite(a, evenC, to_test, max_div_2))
            return false;
    }
    return true;
}



bool RSA_INSTANCE::trialComposite(const mpz_class& a, const mpz_class& evenC, const mpz_class& to_test, int max_div_2) {
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



mpz_class RSA_INSTANCE::calculatePrivateExponent() {
    try {
        mpz_class phi = (PRIME_NUMBER_1 - 1) * (PRIME_NUMBER_2 - 1);
        mpz_class d;

        if (mpz_invert(d.get_mpz_t(), PUBLIC_EXPONENT.get_mpz_t(), phi.get_mpz_t()) == 0) {
            throw std::runtime_error("Error: e does not have an inverse modulo φ(n) in RSA.cpp at line 116");
        }
        return d;
    }
    catch (std::runtime_error& e){
        throw;
    }
}



std::string RSA_INSTANCE::encrypt(const std::string& message) {
    mpz_class m;
    mpz_import(m.get_mpz_t(), message.size(), 1, sizeof(char), 0, 0, message.data());

    mpz_class encrypted;
    mpz_powm(encrypted.get_mpz_t(), m.get_mpz_t(), PUBLIC_EXPONENT.get_mpz_t(), MODULUS.get_mpz_t());

    return encrypted.get_str();
}



std::string RSA_INSTANCE::decrypt(const std::string& ciphertext) {
    mpz_class encrypted(ciphertext);
    mpz_class decrypted;
    mpz_powm(decrypted.get_mpz_t(), encrypted.get_mpz_t(), PRIVATE_EXPONENT.get_mpz_t(), MODULUS.get_mpz_t());

    size_t count = 0;
    auto* decryptedBytes = (unsigned char*) mpz_export(nullptr, &count, 1, sizeof(unsigned char), 0, 0, decrypted.get_mpz_t());

    std::string plaintext(reinterpret_cast<char*>(decryptedBytes), count);

    free(decryptedBytes);
    return plaintext;
}



std::ostream operator<<(std::ostream &out, RSA_INSTANCE &rsa) {
    out << "Generating RSA public key..."
        << "\nPRIME_NUMBER_1: " << RSA_INSTANCE::getFirstPrime()
        << "\nPRIME_NUMBER_1: " << RSA_INSTANCE::getSecondPrime()
        << "\nMODULUS: " << RSA_INSTANCE::getModulus()
        << "\nPRIVATE_EXPONENT: " << RSA_INSTANCE::getPrivateExponent()
        << "\nPUBLIC_EXPONENT: " << 65537
        << std::endl;
    return std::ostream(nullptr);
}



RSA_INSTANCE::~RSA_INSTANCE() {
    delete this;
}


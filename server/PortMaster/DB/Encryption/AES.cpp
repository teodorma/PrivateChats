#include "AES.h"

std::string AES::encrypt(const std::string& aes_key, std::string plaintext) {
    std::cout << aes_key.size() << " " << aes_key << std::endl;
    if (aes_key.size() != 32) {
        throw std::invalid_argument("Invalid AES key length. Must be 32 bytes for AES-256.");
    }

    padData(plaintext);
    const auto* u_aes_key = reinterpret_cast<const unsigned char*>(aes_key.data());

    unsigned char expandedKeys[4 * BLOCK_SIZE * (ROUNDS + 1)];
    KeyExpansion(u_aes_key, expandedKeys);

    auto* u_plaintext = reinterpret_cast<unsigned char*>(&plaintext[0]);
    std::string ciphertext;

    breakIntoChunks(expandedKeys, u_plaintext, plaintext.size(), ciphertext);
    return ciphertext;
}




void AES::breakIntoChunks(unsigned char *expandedKeys, unsigned char *padded_plaintext, size_t padded_len, std::string &ciphertext) {
    for (size_t i = 0; i < padded_len; i += BLOCK_SIZE) {
        unsigned char block[BLOCK_SIZE];
        std::memcpy(block, padded_plaintext + i, BLOCK_SIZE);

        unsigned char state[4][4];
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                state[k][j] = block[j * 4 + k];
            }
        }

        addRoundKey(state, reinterpret_cast<unsigned char (*)[4]>(expandedKeys));

        for (int round = 1; round < ROUNDS; round++) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, reinterpret_cast<unsigned char (*)[4]>(&expandedKeys[round * BLOCK_SIZE * 4]));
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, reinterpret_cast<unsigned char (*)[4]>(&expandedKeys[ROUNDS * BLOCK_SIZE * 4]));

        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                ciphertext += static_cast<char>(state[k][j]);
            }
        }
    }
}



void AES::KeyExpansion(const unsigned char *key, unsigned char *expandedKeys) {
    unsigned char temp[4];
    int i = 0;

    while (i < KEY_SIZE) {
        expandedKeys[i] = key[i];
        i++;
    }

    int numWords = BLOCK_SIZE * (ROUNDS + 1);
    unsigned char rcon = 1;

    while (i < numWords * 4) {
        for (int j = 0; j < 4; j++) {
            temp[j] = expandedKeys[i - 4 + j];
        }

        if (i % KEY_SIZE == 0) {
            unsigned char t = temp[0];
            temp[0] = SUBSTITUTION_BOX[temp[1]];
            temp[1] = SUBSTITUTION_BOX[temp[2]];
            temp[2] = SUBSTITUTION_BOX[temp[3]];
            temp[3] = SUBSTITUTION_BOX[t];

            temp[0] ^= rcon;
            rcon = gmul2(rcon);
        }

        if (i % KEY_SIZE == 16) {
            for (int j = 0; j < 4; j++) {
                temp[j] = SUBSTITUTION_BOX[temp[j]];
            }
        }

        for (int j = 0; j < 4; j++) {
            expandedKeys[i] = expandedKeys[i - KEY_SIZE] ^ temp[j];
            i++;
        }
    }
}



void AES::padData(std::string &data) {
    size_t data_len = data.length();

    size_t padded_len = ((data_len / BLOCK_SIZE) + 1) * BLOCK_SIZE;
    unsigned char padding = padded_len - data_len;

    data.append(padding, static_cast<char>(padding));
}



void AES::addRoundKey(unsigned char (&state)[4][4], unsigned char (*roundKey)[4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] ^= roundKey[i][j];
        }
    }
}



void AES::subBytes(unsigned char (&state)[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] = SUBSTITUTION_BOX[state[i][j]];
        }
    }
}



void AES::shiftRows(unsigned char (&state)[4][4]) {
    unsigned char temp;

    temp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = temp;

    temp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = temp;
    temp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = temp;

    temp = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = temp;
}



void AES::mixColumns(unsigned char (&state)[4][4]) {
    unsigned char temp[4];

    for (int c = 0; c < 4; c++) {
        temp[0] = state[0][c];
        temp[1] = state[1][c];
        temp[2] = state[2][c];
        temp[3] = state[3][c];

        state[0][c] = gmul2(temp[0]) ^ gmul3(temp[1]) ^ temp[2] ^ temp[3];
        state[1][c] = temp[0] ^ gmul2(temp[1]) ^ gmul3(temp[2]) ^ temp[3];
        state[2][c] = temp[0] ^ temp[1] ^ gmul2(temp[2]) ^ gmul3(temp[3]);
        state[3][c] = gmul3(temp[0]) ^ temp[1] ^ temp[2] ^ gmul2(temp[3]);
    }
}



std::string AES::decrypt(const std::string& aes_key, const std::string& ciphertext) {
    if (aes_key.size() != 32) {
        throw std::invalid_argument("Invalid AES key length. Must be 32 bytes for AES-256.");
    }

    const auto* u_aes_key = reinterpret_cast<const unsigned char*>(aes_key.data());

    unsigned char expandedKeys[4 * BLOCK_SIZE * (ROUNDS + 1)];
    KeyExpansion(u_aes_key, expandedKeys);

    auto* u_ciphertext = reinterpret_cast<const unsigned char*>(ciphertext.data());

    std::string plaintext(ciphertext.size(), '\0');

    unsigned char state[4][4];
    for (size_t i = 0; i < ciphertext.size(); i += BLOCK_SIZE) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                state[k][j] = u_ciphertext[i + j * 4 + k];
            }
        }

        addRoundKey(state, reinterpret_cast<unsigned char (*)[4]>(&expandedKeys[ROUNDS * BLOCK_SIZE * 4]));

        for (int round = ROUNDS - 1; round > 0; round--) {
            invShiftRows(state);
            invSubBytes(state);
            addRoundKey(state, reinterpret_cast<unsigned char (*)[4]>(&expandedKeys[round * BLOCK_SIZE * 4]));
            invMixColumns(state);
        }

        invShiftRows(state);
        invSubBytes(state);
        addRoundKey(state, reinterpret_cast<unsigned char (*)[4]>(&expandedKeys[0]));

        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                plaintext[i + j * 4 + k] = state[k][j];
            }
        }
    }

    unsigned char padding = plaintext.back();
    plaintext.resize(plaintext.size() - padding);

    return plaintext;
}



void AES::invShiftRows(unsigned char (&state)[4][4]) {
    unsigned char temp;

    temp = state[1][3];
    state[1][3] = state[1][2];
    state[1][2] = state[1][1];
    state[1][1] = state[1][0];
    state[1][0] = temp;

    temp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = temp;
    temp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = temp;

    temp = state[3][0];
    state[3][0] = state[3][1];
    state[3][1] = state[3][2];
    state[3][2] = state[3][3];
    state[3][3] = temp;
}



void AES::invSubBytes(unsigned char (&state)[4][4]){
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] = INV_SUBSTITUTION_BOX[state[i][j]];
        }
    }
}



void AES::invMixColumns(unsigned char state[4][4]) {
    unsigned char temp[4];

    for (int i = 0; i < 4; i++) {
        temp[0] = state[0][i];
        temp[1] = state[1][i];
        temp[2] = state[2][i];
        temp[3] = state[3][i];

        state[0][i] = gmul(temp[0], 0x0e) ^ gmul(temp[1], 0x0b) ^ gmul(temp[2], 0x0d) ^ gmul(temp[3], 0x09);
        state[1][i] = gmul(temp[0], 0x09) ^ gmul(temp[1], 0x0e) ^ gmul(temp[2], 0x0b) ^ gmul(temp[3], 0x0d);
        state[2][i] = gmul(temp[0], 0x0d) ^ gmul(temp[1], 0x09) ^ gmul(temp[2], 0x0e) ^ gmul(temp[3], 0x0b);
        state[3][i] = gmul(temp[0], 0x0b) ^ gmul(temp[1], 0x0d) ^ gmul(temp[2], 0x09) ^ gmul(temp[3], 0x0e);
    }
}



unsigned char AES::gmul(unsigned char a, unsigned char b) {
    unsigned char p = 0;
    unsigned char hi_bit_set;
    for (int counter = 0; counter < 8; counter++) {
        if (b & 1) {
            p ^= a;
        }
        hi_bit_set = (a & 0x80);
        a <<= 1;
        if (hi_bit_set) {
            a ^= 0x1b;
        }
        b >>= 1;
    }
    return p;
}



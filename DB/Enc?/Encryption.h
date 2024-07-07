//
// Created by maciucateodor on 6/25/24.
//

#ifndef SERVER_ENCRYPTION_H
#define SERVER_ENCRYPTION_H
#include <vector>
#include <random>
#include "Number.h"

class Encryption{
private:
    static int KEY_LENGTH;
    Number KEY1;
    Number KEY2;
public:
    explicit Encryption(){}

    genKEY(){
        int bits = nBitRandom(KEY_LENGTH);
    }


};


#endif //SERVER_ENCRYPTION_H

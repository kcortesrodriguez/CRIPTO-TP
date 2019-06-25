#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "random.h"
#include "modular.h"
#include <limits.h>

#define TRUE 0
#define FALSE 1

//chequea si un elemento value se encuentra en el array array de longitud sizeArray
int containsValue(const uint8_t *array, int sizeArray, int value) {
    for (int i = 0; i < sizeArray; i++) {
        if (array[i] == value) {
            return TRUE;
        }
    }
    return FALSE;
}

//genera un vector de n randoms en Z 251 que no se repiten
uint8_t *generateRandoms(int n) {
    uint8_t *array = (uint8_t *) calloc((size_t) n, sizeof(uint8_t));

    uint8_t randomNumber;

    for (int i = 0; i < n; i++) {
        do {
            randomNumber = safe_next_char();
        } while (containsValue(array, i, randomNumber) == TRUE);

        array[i] = randomNumber;
    }

    return array;
}

void set_seed(int64_t s) {
    seed = (s ^ 0x5DEECE66DL) & ((1LL << 48) - 1);
}

uint8_t next_char(void) {
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1LL << 48) - 1);
    return (uint8_t) (seed >> 40);
}

uint8_t safe_next_char() {
    uint8_t res = next_char();
    while (res >= 251) {
        res = next_char();
    }
    return res;
}


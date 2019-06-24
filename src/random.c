#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "random.h"
#include "modular.h"
#include <limits.h>

#define TRUE 0
#define FALSE 1

int urandom_fd = -2;

void urandom_init() {
    urandom_fd = open("/dev/urandom", O_RDONLY);

    if (urandom_fd == -1) {
        int errsv = urandom_fd;
        printf("Error opening [/dev/urandom]: %i\n", errsv);
        exit(1);
    }
}

unsigned long urandom() {
    unsigned long buf_impl;
    unsigned long *buf = &buf_impl;

    if (urandom_fd == -2) {
        urandom_init();
    }

    /* Read 4 bytes, or 32 bits into *buf, which points to buf_impl */
    read(urandom_fd, buf, sizeof(long));
    return buf_impl;
}

//chequea si un elemento value se encuentra en el array array de longitud sizeArray
int containsValue(const int *array, int sizeArray, int value) {
    for (int i = 0; i < sizeArray; i++) {
        if (array[i] == value) {
            return TRUE;
        }
    }
    return FALSE;
}

//genera un vector de n randoms en Z 251 que no se repiten
int *generateRandoms(int n) {
    int *array = (int *) malloc(sizeof(int) * n);

    for (int i = 0; i < n; i++) {

        int randomNumber = (int) modulo(safe_next_char(), 251);
//        int randomNumber = (int) modulo(urandom(), 251);

        if (i != 0) {
            do {
                randomNumber = (int) modulo(safe_next_char(), 251);
//                randomNumber = (int) modulo(urandom(), 251);
            } while (containsValue(array, i, randomNumber) == TRUE);
        }

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

uint8_t safe_next_char(void) {
    uint8_t res = next_char();
    while (res >= 251) {
        res = next_char();
    }
    return res;
}


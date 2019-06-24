#ifndef CRIPTO_TP_RANDOM_H
#define CRIPTO_TP_RANDOM_H

unsigned long urandom();

int *generateRandoms(int n);

/**
 * variable global
 * seed debe ser de 48 bits; se elige este tipo de 64 bits
 */
int64_t seed;

void set_seed(int64_t seed);

/**
 * devuelve un unsigned char
 */
uint8_t next_char(void);

uint8_t safe_next_char(void);

#endif //CRIPTO_TP_RANDOM_H

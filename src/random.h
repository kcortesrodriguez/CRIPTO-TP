#ifndef CRIPTO_TP_RANDOM_H
#define CRIPTO_TP_RANDOM_H

uint8_t * generateRandoms(int n);

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

uint8_t safe_next_char();

#endif //CRIPTO_TP_RANDOM_H

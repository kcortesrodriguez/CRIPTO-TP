#include <stdio.h>
#include <stdlib.h>
#include "global.h"

// Considering negative x too
long modulo(long x, int N) {
    return (x % N + N) % N;
}

// Function to calculate modular inverse from 1 to n
int *modularInverse(int n, int prime) {
    int *inverses = (int *) malloc(prime * sizeof(int));

    inverses[0] = -1; // does not exist for 0
    inverses[1] = 1;
    for (int i = 2; i <= n; i++)
        inverses[i] = inverses[prime % i] * (prime - prime / i) % prime;

    return inverses;
}

uint32_t mul_mod(uint32_t a, uint32_t b, uint32_t mod) {
    long double x;
    uint32_t c;
    int32_t r;
    if (a >= mod)
        a %= mod;
    if (b >= mod)
        b %= mod;
    x = a;
    c = x * b / mod;
    r = (int32_t) (a * b - c * mod) % (int32_t) mod;
    return r < 0 ? r + mod : r;
}


#include <stdio.h>
#include <stdlib.h>

// Considering negative x too
// TODO remember to include this discovery in report
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


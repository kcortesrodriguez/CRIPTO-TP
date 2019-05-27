#include <stdio.h>

// Function to calculate modular inverse from 1 to n
void modularInverse(int n, int prime, int *a) {
    a[0] = a[1] = 1;
    for (int i = 2; i <= n; i++)
        a[i] = a[prime % i] * (prime - prime / i) % prime;
}


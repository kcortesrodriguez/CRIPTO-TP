#include <stdio.h>
#include "modular.h"

int main() {

    int inverses[251];

    // Populate multiplicative inverses mod 251
    modularInverse(250, 251, inverses);

//    for (int i = 0; i < 250; i++) {
//        printf("*(a + %d) : %d\n", i, *(inverses + i));
//    }

    return 0;
}
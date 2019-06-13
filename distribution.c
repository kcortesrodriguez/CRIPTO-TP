#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "random.h"
#include "distribution.h"
#include "matrix.h"
#include "modular.h"

// rank k
// det != 0
// proj(A) y S - proj(A) no debe tener valores mayores que 251
long **matA(int n, int k) {
    long **A = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) A[i] = (long *) malloc(k * sizeof(long));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            A[i][j] = (long) (urandom() % 251); // 251 because A must have values in [0, 251)
        }
    }

    // Paper case
    A[0][0] = 10;
    A[0][1] = 1;
    A[1][0] = 7;
    A[1][1] = 2;
    A[2][0] = 8;
    A[2][1] = 4;
    A[3][0] = 1;
    A[3][1] = 1;

    printf("A matrix:\n");
    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < k; columns++) {
            printf("  %d", A[row][columns]);
        }
        printf("\n");
    }

    return A;
}

long **projectionSd(long **A, int n, int k, int inverses[251]) {

    long **At = transpose(A, n, k);

    printf("\n");

    printf("At matrix:\n");
    for (int row = 0; row < k; row++) {
        for (int columns = 0; columns < n; columns++) {
            printf("  %ld", At[row][columns]);
        }
        printf("\n");
    }

    long **AtA = multiply(At, A, k, k, n);

    printf("\n");
    printf("pre AtA matrix:\n");
    for (int row = 0; row < k; row++) {
        for (int columns = 0; columns < k; columns++) {
            printf("  %ld", AtA[row][columns]);
        }
        printf("\n");
    }

    int det = determinantOfMatrix(AtA, k, k);

    printf("post AtA matrix:\n");
    for (int row = 0; row < k; row++) {
        for (int columns = 0; columns < k; columns++) {
            printf("  %ld", AtA[row][columns]);
        }
        printf("\n");
    }

    printf("\n");
    printf("det: %d\n", det);

    if (det != 0) {
        printf("%s", "Inverse exists!\n");

        long **AugmentedAtaInverse = inverse(AtA, k, inverses);

        freeMatrix(AtA, k);

        printf("\n");
        printf("AugmentedAtaInverse matrix:\n");
        for (int row = 0; row < k; row++) {
            for (int columns = 0; columns < k * 2; columns++) {
                printf("  %ld", AugmentedAtaInverse[row][columns]);
            }
            printf("\n");
        }

        long **AtAInverse = (long **) malloc(k * sizeof(long *));
        for (int i = 0; i < k; i++) AtAInverse[i] = (long *) calloc((size_t) k, sizeof(long));

        // Fill AtAInverse
        for (int row = 0; row < k; row++) {
            for (int columns = 0; columns < k; columns++) {
                AtAInverse[row][columns] = AugmentedAtaInverse[row][columns + k];
            }
        }

        printf("\n");
        printf("AtAInverse matrix:\n");
        for (int row = 0; row < k; row++) {
            for (int columns = 0; columns < k; columns++) {
                printf("  %ld", AtAInverse[row][columns]);
            }
            printf("\n");
        }

        freeMatrix(AugmentedAtaInverse, k);

        // A * (At * A)'

        long **AxInverse = multiply(A, AtAInverse, n, k, k);

        freeMatrix(AtAInverse, k);

        printf("\n");
        printf("AxInverse matrix:\n");
        for (int row = 0; row < n; row++) {
            for (int columns = 0; columns < k; columns++) {
                printf("  %ld", AxInverse[row][columns]);
            }
            printf("\n");
        }

        // (A * (At * A)') * At


        long **proj = multiply(AxInverse, At, n, n, k);

        printf("Entroooo");

        //freeMatrix(AxInverse, n);
        //freeMatrix(At, k);

        return proj;
    } else {
        printf("%s", "Inverse does not exist!");

        return NULL;
    }
}

long **remainderR(long **secretS, long **projectionSd, int n) {
    return subtract(secretS, projectionSd, n);
}

long **remainderRw(long **watermarkW, long **projectionSd, int n) {
    return subtract(watermarkW, projectionSd, n);

}
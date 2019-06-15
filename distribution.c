#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
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
    A[0][0] = 3;
    A[0][1] = 7;
    A[1][0] = 6;
    A[1][1] = 1;
    A[2][0] = 2;
    A[2][1] = 5;
    A[3][0] = 6;
    A[3][1] = 6;

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

/*
 * R is the remainder matrix.
 * c is the coefficents array (from 1 to n).
 * n is the max number of participants.
 * k is the min number of participants.
 * t is the current participant index.
 */
int *g_i_j(int **R, int initial_column, int t, int n, int k) {
    int *res = (int *) calloc(n, sizeof(int));

    for (int row = 0; row < n; row++) {
        for (int column = initial_column; column < k + initial_column; column++) {
            res[row] += R[row][column] * pow(t, column - initial_column);
        }
        res[row] = modulo(res[row], 251);
    }

    return res;
}

/*
 * R is the remainder matrix.
 * n is the max number of participants.
 * k is the min number of participants.
 * t is the current participant index.
 */
int **matG_t(int **R, int n, int k, int t) {
    int max_t = (int) ceil(n / k); // TODO: code method for create matrix !
    int **res = (int **) calloc(max_t, sizeof(int *));

    for (int i = 0; i < max_t; i++) {
        res[i] = g_i_j(R, i * 2, t, n, k);
    }

    int **resT = (int **) malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) resT[i] = (int *) calloc(max_t, sizeof(int));
    transpose(res, resT, max_t, n);

    freeMatrix(res, max_t);

    return resT;
}

/*
 * R is the remainder matrix.
 * n is the max number of participants.
 * k is the min number of participants.
 */
int ***matG(int **R, int n, int k) {
    int ***matG = (int ***) malloc(n * sizeof(int **));

    for (int t = 0; t < n; t++) {
        matG[t] = matG_t(R, n, k, t + 1);
    }

    return matG;
}
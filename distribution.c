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
            A[i][j] = modulo(urandom(), 251); // 251 because A must have values in [0, 251)
        }
    }

    return A;
}

long **projectionSd(long **A, int n, int k, int inverses[251]) {

    long **At = transpose(A, n, k);

//    printf("\nAt matrix:\n");
//    printMatrix(n, k, At);

    long **AtA = multiply(At, A, k, k, n);

    long det = determinantOfMatrix(AtA, k, k);

    if (det != 0) {
        long **AugmentedAtaInverse = inverse(AtA, k, inverses);

//        printf("\nAugmentedAtaInverse matrix:\n");
//        printMatrix(k * 2, k, AugmentedAtaInverse);

        long **AtAInverse = (long **) malloc(k * sizeof(long *));
        for (int i = 0; i < k; i++) AtAInverse[i] = (long *) calloc((size_t) k, sizeof(long));

        // Fill AtAInverse
        for (int row = 0; row < k; row++) {
            for (int columns = 0; columns < k; columns++) {
                AtAInverse[row][columns] = AugmentedAtaInverse[row][columns + k];
            }
        }

//        printf("\nAtAInverse matrix:\n");
//        printMatrix(k, k, AtAInverse);

        // A * (At * A)'
        long **AxInverse = multiply(A, AtAInverse, n, k, k);

//        printf("\nAxInverse matrix:\n");
//        printMatrix(k, n, AxInverse);

        // (A * (At * A)') * At
        long **proj = multiply(AxInverse, At, n, n, k);

        freeMatrix(AugmentedAtaInverse, k);
        freeMatrix(AtA, k);
        freeMatrix(AtAInverse, k);
        freeMatrix(AxInverse, n);
        freeMatrix(At, k);

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
long *g_i_j(long **R, int initial_column, int t, int n, int k) {
    long *res = (long *) calloc(n, sizeof(long));

    for (int row = 0; row < n; row++) {
        for (int column = initial_column; column < k + initial_column; column++) {
            res[row] += R[row][column] * pow((double) t, (double) (column - initial_column));
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
long **matG_t(long **R, int n, int k, int t) {
    int max_t = (int) ceil((double) n / k); // TODO: code method for create matrix !
    long **res = (long **) calloc(max_t, sizeof(long *));

    for (int i = 0; i < max_t; i++) {
        res[i] = g_i_j(R, i * 2, t, n, k);
    }

    long **resT = transpose(res, max_t, n);

    freeMatrix(res, max_t);

    return resT;
}

/*
 * R is the remainder matrix.
 * n is the max number of participants.
 * k is the min number of participants.
 */
long ***matG(long **R, int n, int k) {
    long ***matG = (long ***) malloc(n * sizeof(long **));

    for (int t = 0; t < n; t++) {
        matG[t] = matG_t(R, n, k, t + 1);
    }

    return matG;
}

long **matV(long **A, long **X, int n, int k) {
    long **matrixMultiply;
    printf("\n");
    matrixMultiply = multiply(A, X, n, n, k);

    return matrixMultiply;
}

long ***matSh(long ***G, long **V, int n, int k) {
    long ***Sh = (long ***) malloc(n * sizeof(long **));

    for (int i = 0; i < n; i++) {
        Sh[i] = concat(V[i], G[i], n, k);
    }

    return Sh;
}

long *generateVector(int k, int initialValue) {
    long *array = (long *) malloc(sizeof(long) * k);

    for (int i = 0; i < k; i++) {
        array[i] = modulo((long) pow((double) initialValue, (double) i), 251);
    }

    return array;
}

/*
 * Generate matrix X.
 * N vector of dimensions k x 1, linearly independent.
 * k columns
 * n rows
 */
long **matX(int k, int n) {
    long **temp = (long **) malloc(n * sizeof(long *));
    int *randoms = generateRandoms(n);

    for (int i = 0; i < n; i++) {
        temp[i] = generateVector(k, randoms[i]);
    }

    return transpose(temp, n, k);
}


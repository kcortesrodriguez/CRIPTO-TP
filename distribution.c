//
//
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "random.h"
#include "distribution.h"
#include "matrix.h"

// rank k
// det != 0
// proj(A) y S - proj(A) no debe tener valores mayores que 251
int **matA(int n, int k) {
    int **A = (int **) malloc(n * sizeof(int *)); //TODO free
    for (int i = 0; i < n; i++) A[i] = (int *) malloc(k * sizeof(int));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            A[i][j] = (int) (urandom() % 251); // 251 because A must have values in [0, 251)
        }
    }

    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < k; columns++) {
            printf("  %d", A[row][columns]);
        }
        printf("\n");
    }

    int **At = (int **) malloc(k * sizeof(int *)); //TODO free
    for (int i = 0; i < k; i++) At[i] = (int *) malloc(n * sizeof(int));

    transpose(A, At, n, k);

    int **AtA = (int **) malloc(k * sizeof(int *)); //TODO free
    for (int i = 0; i < k; i++) AtA[i] = (int *) malloc(k * sizeof(int));

    printf("\n");
    printf("\n");
    printf("\n");

    for (int row = 0; row < k; row++) {
        for (int columns = 0; columns < n; columns++) {
            printf("  %d", At[row][columns]);
        }
        printf("\n");
    }

    multiply(At, A, AtA, k, k, n);

    printf("\n");
    printf("\n");
    printf("\n");

    for (int row = 0; row < k; row++) {
        for (int columns = 0; columns < k; columns++) {
            printf("  %d", AtA[row][columns]);
        }
        printf("\n");
    }

    int det = determinantOfMatrix(AtA, k, k);

    printf("det: %d\n", det);

    if (det != 0) {
        printf("%s", "Inverse exists!");
    } else {
        printf("%s", "Inverse does not exist!");
    }

    return A;
}
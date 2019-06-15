#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "modular.h"
#include "io.h"
#include "distribution.h"
#include "matrix.h"

#define MAX_LEN 100

int main(int argc, char *argv[]) {

    char secretImage[MAX_LEN];
    char retrievedImage[MAX_LEN];
    char watermarkImage[MAX_LEN];
    char watermarkTransformationImage[MAX_LEN];
    char directory[MAX_LEN];
    int k;
    int n;

    // Parse parameters
    // e.g.: -d -s Albert.bmp -m Paris.bmp -k 4 -n 8 --dir color280x440/
    parseParameters(argc, argv,
                    sizeof secretImage / sizeof *secretImage,
                    secretImage, retrievedImage,
                    watermarkImage, watermarkTransformationImage,
                    directory,
                    &k, &n);

    // Populate multiplicative inverses mod 251
    int *inverses = modularInverse(250, 251);

    // Matrix A
    long **A = matA(n, k);

    // Matrix Sd
    long **Sd = projectionSd(A, n, k, inverses);

    printf("\n");
    printf("Sd matrix:\n");
    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < n; columns++) {
            printf("  %ld", Sd[row][columns]);
        }
        printf("\n");
    }

    long **S = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) S[i] = (long *) calloc(n, sizeof(long));

    // Paper case
    S[0][0] = 2L;
    S[0][1] = 5L;
    S[0][2] = 2L;
    S[0][3] = 3L;

    S[1][0] = 3L;
    S[1][1] = 6L;
    S[1][2] = 4L;
    S[1][3] = 5L;

    S[2][0] = 4L;
    S[2][1] = 7L;
    S[2][2] = 4L;
    S[2][3] = 6L;

    S[3][0] = 1L;
    S[3][1] = 4L;
    S[3][2] = 1L;
    S[3][3] = 7L;

    // Matrix R
    long **R = remainderR(S, Sd, n);

    printf("\n");
    printf("R matrix:\n");
    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < n; columns++) {
            printf("  %ld", R[row][columns]); //TODO write in report that R matrix from paper is wrong.q
        }
        printf("\n");
    }

    // Recovered Matrix S
    long **recoveredS = add(R, Sd, n);
    printf("\n");
    printf("S matrix:\n");
    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < n; columns++) {
            printf("  %ld", recoveredS[row][columns]);
        }
        printf("\n");
    }

    // Matrix Rw
//    long **Rw = remainderRw(W, Sd, n); //todo: replace W with watermark

    free(inverses);
    freeMatrix(A, n);
    freeMatrix(Sd, n);

    freeMatrix(recoveredS, n);
    freeMatrix(S, n);


    // Start testing Matrix X (Kevin)
//    k = 4;
//    n = 8;

//    long** matrix;
//    printf("\n");
//    matrix = generateMatrixX(k,n);
//    printMatrix(k,n,matrix);
//
//    long** matrixTranspose;
//    matrixTranspose = transposeV2(matrix,k,n); //n x k
//    printMatrix(n,k,matrixTranspose); // n x k

    //long** matrixMultiply;
    //matrixMultiply = multiplyV2(matrix, matrixMultiply, k, n, k);
    //printMatrix(k, k, matrixMultiply); // k x k


    // Matrix G
    printf("\n");
    printf("G matrix:\n");
    long ***G = matG(R, n, k);
    for (int t = 0; t < n; t++) {
        printf("G_%d matrix:\n", t + 1);
        for (int row = 0; row < n; row++) {
            for (int columns = 0; columns < k; columns++) {
                printf("  %ld", G[t][row][columns]);
            }
            printf("\n");
        }
    }

    freeMatrix(R, n);

    for (int t = 0; t < n; t++) {
        freeMatrix(G[t], n);
    }

    free(G);

    return 0;
}

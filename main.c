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
            printf("  %d", Sd[row][columns]);
        }
        printf("\n");
    }

    long **S = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) S[i] = (long *) malloc(k * sizeof(long));

    // Paper case
    S[0][0] = 2;
    S[0][1] = 3;
    S[0][2] = 1;
    S[0][3] = 2;

    S[1][0] = 5;
    S[1][1] = 4;
    S[1][2] = 6;
    S[1][3] = 1;

    S[2][0] = 8;
    S[2][1] = 9;
    S[2][2] = 7;
    S[2][3] = 2;

    S[3][0] = 3;
    S[3][1] = 4;
    S[3][2] = 1;
    S[3][3] = 2;

    // Matrix R
    long **R = remainderR(S, Sd, n);

    printf("\n");
    printf("R matrix:\n");
    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < n; columns++) {
            printf("  %d", R[row][columns]); //TODO write in report that R matrix from paper is wrong.q
        }
        printf("\n");
    }

    // Recovered Matrix S
    long **recoveredS = add(R, Sd, n);
    printf("\n");
    printf("S matrix:\n");
    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < n; columns++) {
            printf("  %d", recoveredS[row][columns]);
        }
        printf("\n");
    }

    // Matrix Rw
//  int **Rw = remainderRw(W, Sd, n); //todo: replace W with watermark

    free(inverses);
    freeMatrix(A, n);
    freeMatrix(Sd, n);
    freeMatrix(R, n);

    freeMatrix(recoveredS, n);
    freeMatrix(S, n);


    // Start testing Matrix X (Kevin)
    k = 4;
    n = 8;

    long** matrix;
    printf("\n");
    matrix = generateMatrixX(k,n);
    printMatrix(k,n,matrix);

    long** matrixTranspose;
    matrixTranspose = transposeV2(matrix,k,n); //n x k
    printMatrix(n,k,matrixTranspose); // n x k

    //long** matrixMultiply;
    //matrixMultiply = multiplyV2(matrix, matrixMultiply, k, n, k);
    //printMatrix(k, k, matrixMultiply); // k x k

    return 0;
}
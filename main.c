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
    // int **Rw = remainderRw(W, Sd, n); //todo: replace W with watermark

    //free(inverses);
    //freeMatrix(A, n);
    //freeMatrix(Sd, n);
    //freeMatrix(R, n);

    //freeMatrix(recoveredS, n);
    //freeMatrix(S, n);

    // Start testing Matrix X (Kevin)

    k = 2;
    n = 4;

    long** matrix;
    printf("\n");
    matrix = generateMatrixX(k,n); //filas x columnas --> n filas x k columnas
    printMatrix(k,n,matrix);

    printf("\n");
    printMatrix(k,n,A);

    long** matrixTranspose;
    printf("\n");
    matrixTranspose = transposeV2(matrix,k,n); //filas x columnas --> k filas x n columnas
    printMatrix(n,k,matrixTranspose); // n x k

    //Tenemos que hacer A x X para tener V
    // A --> n x k
    // MI X --> n x k, entonces lo hago con la Xt --> k x n
    // A x Xt --> n x n

    long** matrixMultiply;
    printf("\n");
    matrixMultiply = multiplyV2(A, matrixTranspose, n, n, k);
    printMatrix(n, n, matrixMultiply); // k x k


    return 0;
}
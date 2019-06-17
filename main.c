#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "modular.h"
#include "io.h"
#include "distribution.h"
#include "matrix.h"
#include "global.h"

int VERBOSE = FALSE;

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

    printMatrix(n,n, Sd, "Sd matrix");

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

    printMatrix(n,n,R, "R matrix"); //TODO write in report that R matrix from paper is wrong.q

    // Recovered Matrix S
    long **recoveredS = add(R, Sd, n);
    printMatrix(n,n,recoveredS, "S matrix");

    // Matrix Rw
//    long **Rw = remainderRw(W, Sd, n); //todo: replace W with watermark

    //free(inverses);
    //freeMatrix(A, n);
    freeMatrix(Sd, n);

    freeMatrix(recoveredS, n);
    freeMatrix(S, n);

    //freeMatrix(recoveredS, n);
    //freeMatrix(S, n);

    // Start testing Matrix X (Kevin)

    k = 2;
    n = 4;

    long **matrix;
    matrix = matX(k, n); //filas x columnas --> n filas x k columnas
    printMatrix(n, k, matrix, "X matrix");

    printMatrix(k, n, A, "A matrix");


    //Tenemos que hacer A x X para tener V
    // A --> n x k
    // MI X --> n x k, entonces lo hago con la Xt --> k x n
    // A x Xt --> n x n

    long **V = matV(A, matrix, n, k);
    printMatrix(n, n, V, "V matrix:"); // k x k

    long **VDemo = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) VDemo[i] = (long *) calloc(n, sizeof(long));

    // Paper case
    VDemo[0][0] = 62L;
    VDemo[0][1] = 59L;
    VDemo[0][2] = 43L;
    VDemo[0][3] = 84L;

    VDemo[1][0] = 40L;
    VDemo[1][1] = 28L;
    VDemo[1][2] = 28L;
    VDemo[1][3] = 48L;

    VDemo[2][0] = 83L;
    VDemo[2][1] = 62L;
    VDemo[2][2] = 58L;
    VDemo[2][3] = 102L;

    VDemo[3][0] = 23L;
    VDemo[3][1] = 20L;
    VDemo[3][2] = 16L;
    VDemo[3][3] = 30L;

    // Matrix G
    long ***G = matG(R, n, k);
    for (int t = 0; t < n; t++) {
        printMatrix(k,n,G[t], "G_ matrix");
    }

    freeMatrix(R, n);

    // Matrix Sh
    long ***Sh = matSh(G, VDemo, n, k);
    for (int t = 0; t < n; t++) {
        printMatrix(k+1,n,Sh[t], "Sh_ matrix");
    }

    for (int t = 0; t < n; t++) {
        freeMatrix(G[t], n);
    }
    free(G);
//    free(Sh);


    // 4.3.2
    // Por cada Sh desconcatenamos V_t y G_t
    long** recoveredG;
    long* recoveredV;
    for (int l = 0; l < n; ++l) {
        recoveredG = deconcatG(Sh[l], n, k);
        recoveredV = deconcatV(Sh[l], n);
        printMatrix(k,n,recoveredG, "Recovered G");
        printVector(n,recoveredV,"Recovered V");
        // Luego de cada G_t sacamos la R_t
    }


    //Start desencryption example from paper

    long **B = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) B[i] = (long *) calloc(k, sizeof(long));

    B[0][0] = 62;
    B[0][1] = 40;
    B[1][0] = 59;
    B[1][1] = 28;
    B[2][0] = 43;
    B[2][1] = 28;
    B[3][0] = 84;
    B[3][1] = 48;

    printMatrix(k,n,B, "B Matrix");

    long ** newSd = projectionSd(B, n, k, inverses);

    printMatrix(n,n,newSd, "Reconstruction Sd Matrix from B Matrix:");

    return EXIT_SUCCESS;
}

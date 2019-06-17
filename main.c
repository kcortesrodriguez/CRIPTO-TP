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
    printf("\nX matrix:\n");
    matrix = matX(k, n); //filas x columnas --> n filas x k columnas
    printMatrix(n, k, matrix);

    printf("\n");
    printMatrix(k, n, A);


    //Tenemos que hacer A x X para tener V
    // A --> n x k
    // MI X --> n x k, entonces lo hago con la Xt --> k x n
    // A x Xt --> n x n

    long **V = matV(A, matrix, n, k);
    printf("\nV matrix:\n");
    printMatrix(n, n, V); // k x k

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

    // Matrix Sh
    printf("\n");
    printf("Sh matrix:\n");
    long ***Sh = matSh(G, VDemo, n, k);
    for (int t = 0; t < n; t++) {
        printf("Sh_%d matrix:\n", t + 1);
        for (int row = 0; row < n; row++) {
            for (int columns = 0; columns < k + 1; columns++) {
                printf("  %ld", Sh[t][row][columns]);
            }
            printf("\n");
        }
    }

    for (int t = 0; t < n; t++) {
        freeMatrix(G[t], n);
    }
    free(G);
    free(Sh);

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

    printf("\nB Matrix:\n");

    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < k; columns++) {
            printf("  %ld", B[row][columns]);
        }
        printf("\n");
    }

    long ** newSd = projectionSd(B, n, k, inverses);

    printf("\nReconstruction Sd Matrix from B Matrix: \n");

    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < n; columns++) {
            printf("  %ld", newSd[row][columns]);
        }
        printf("\n");
    }

    return 0;
}

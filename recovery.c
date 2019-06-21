#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "modular.h"
#include "global.h"
#include "recovery.h"


uint8_t reverse(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

long *gaussJordan(int n, long **matrix, int inverses[251]) {

    //Genero una copia de la matriz matrix, pero en floats para operar sobre esta
    float **a = (float **) malloc(n * sizeof(float *));
    for (int i = 0; i < n; i++)
        a[i] = (float *) calloc(n + 1, sizeof(float));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n + 1; j++) {
            a[i][j] = matrix[i][j] * 1.0;
        }
    }

//    for(int i = 0 ; i < n ; i++){
//        for(int j = 0 ; j < n + 1 ; j++){
//            printf("%f", a[i][j]);
//        }
//        printf("\n");
//    }

    int kprime = n + 1;
    float t;
    int i, j, k;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i != j) {
                t = modulo(a[j][i] * inverses[(long) a[i][i]], 251);
                for (k = 0; k < kprime; k++)
                    a[j][k] = modulo(a[j][k] - modulo(a[i][k] * t, 251), 251);
            }
        }
    }

    //Hago un vector para devolver los valores en long!
    long *solutionVector = calloc(n, sizeof(long));

    for (i = 0; i < n; i++) {
        solutionVector[i] = modulo(a[i][n] * inverses[(long) a[i][i]],
                                   251); //TODO: like inverses in matrix., receive inverses
        //printf("%f \n", a[i][n] / a[i][i]);
        //printf("%ld \n", solutionVector[i]);
    }

    return solutionVector;

}

//devuelve una matriz de j x 2
long **matrixCj(int j) {

    long **a = (long **) malloc(j * sizeof(long *));
    for (int i = 0; i < j; i++)
        a[i] = (long *) calloc(2, sizeof(long));

    for (int i = 0; i < j; i++) {
        a[i][0] = 1; // TODO: from
        a[i][1] = i + 1; //TODO: to (remember case where cj = 3 && cj = 4
    }

    /*
    for (int i = 0 ; i < j ; i++){
        for (int s = 0 ; s < 2 ; s++){
            printf("a[%d][%d] = %ld\t", i, s, a[i][s]);
        }
        printf("\n");
    }
     */

    return a;

}

//devuelve una matriz de 2 x 1. totalGs es el equivalente al j de matrixCj, que es k
// allGs tiene k matrices G, aunque se generen n en distribution
long **resultG(int x, int y, long ***allGs, int totalGs) {

    long **returnMatrix = (long **) malloc(totalGs * sizeof(long *));
    for (int i = 0; i < totalGs; i++)
        returnMatrix[i] = (long *) calloc(1, sizeof(long));

    for (int i = 0; i < totalGs; i++) {
        long **G_i = allGs[i];
        returnMatrix[i][0] = G_i[x][y];
    }

    return returnMatrix;
}

//int main() {
//
//    matrixCj(6);
//
//    int n = 2;
//
//    long **a = (long **) malloc(n * sizeof(long *));
//    for (int i = 0; i < n; i++)
//        a[i] = (long *) calloc(n + 1, sizeof(long));
//
//    a[0][0] = 1;
//    a[0][1] = 1;
//    a[0][2] = 36;
//    a[1][0] = 1;
//    a[1][1] = 2;
//    a[1][2] = 71;
//
//    /*
//    for(int i = 0 ; i < n ; i++){
//        for(int j = 0 ; j < n + 1 ; j++){
//            printf("%ld\t", a[i][j]);
//        }
//        printf("\n");
//    }
//     */
//
//    int *inverses = modularInverse(250, 251);
//
//    long *solutionVector = gaussJordan(n, a, inverses);
//
//    /*
//    for (int i = 0; i < n; i++) {
//        printf("%ld \n", solutionVector[i]);
//    }
//     */
//
//}


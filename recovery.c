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
    long **a = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++)
        a[i] = (long *) calloc(((size_t) n + 1), sizeof(long));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < (n + 1); j++) {
            a[i][j] = matrix[i][j];
        }
    }

    int kprime = n + 1;
    long t;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j) {
                t = modulo(a[j][i] * inverses[a[i][i]], 251);
                for (int k = 0; k < kprime; k++)
                    a[j][k] = modulo(a[j][k] - modulo(a[i][k] * t, 251), 251);
            }
        }
    }

    //Hago un vector para devolver los valores en long!
    long *solutionVector = calloc((size_t) n, sizeof(long));

    for (int i = 0; i < n; i++) {
        solutionVector[i] = modulo(a[i][n] * inverses[a[i][i]],
                                   251); //TODO: like inverses in matrix., receive inverses
        printf("\nsolutionVector[i]:\t%ld \n", solutionVector[i]);
    }

    return solutionVector;
}

//devuelve una matriz de j x 2
//DEPRECATED
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

//devuelve la matriz de j x k
long **matrixCjV2(int j, int k) {

    long **a = (long **) malloc(j * sizeof(long *));
    for (int i = 0; i < j; i++)
        a[i] = (long *) calloc(k, sizeof(long));

    for (int i = 0; i < j; i++) {
        for (int m = 0; m < k; m++) {
            //a[i][m] = (i+1)**m
            a[i][m] = modulo((long) pow((double) (i + 1), (double) m), 251);
        }
    }

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


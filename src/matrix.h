#include "global.h"

#ifndef CRIPTO_TP_MATRIX_H
#define CRIPTO_TP_MATRIX_H

long determinantOfMatrix(long **mat, int N, int n);

long **transpose(long **A, int n, int k);

long **multiply(long **mat1, long **mat2, int n, int m, int k);

long **inverse(long **m, int n, int inverses[251]);

long **add(long **mat1, long **mat2, int n);

long **subtract(long **mat1, long **mat2, int n);

void freeLongMatrix(long **m, int n);

void freeCharMatrix(char **m, int n);

void printVector(int k, long *array, char *title);

void printMatrix(int k, int n, long **matrix, char *title);

long **concatVecMat(long *vec, long **mat, int n, int k);

long **concatMatMat(long **mat1, long **mat2, int rows, int cols1, int cols2);

uint8_t **convertMatrixFromLongToUint8(long **mat, int n, int k);

long **convertUint8StreamToLongMatrix(uint8_t *stream, int n, int k);

long **deconcatG(long **mat, int n, int k);

long *deconcatV(long **mat, int n);

#endif //CRIPTO_TP_MATRIX_H

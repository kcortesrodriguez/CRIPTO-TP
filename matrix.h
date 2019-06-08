//
//

#ifndef CRIPTO_TP_MATRIX_H
#define CRIPTO_TP_MATRIX_H

int determinantOfMatrix(int **mat, int N, int n);

void transpose(int **A, int **B, int n, int k);

void multiply(int **mat1, int **mat2, int **res, int n, int m, int k);

long* generateVector(int k, int initialValue);

void printVector(int k, long* array);

long** generateMatrixX(int k, int n);

void printMatrix(int k, int n, long** matrix);

#endif //CRIPTO_TP_MATRIX_H

#ifndef CRIPTO_TP_MATRIX_H
#define CRIPTO_TP_MATRIX_H

int determinantOfMatrix(int **mat, int N, int n);

void transpose(int **A, int **B, int n, int k);

void multiply(int **mat1, int **mat2, int **res, int n, int m, int k);

void inverse(int **m, int **mInverse, int n, int inverses[251]);

void add(int **mat1, int **mat2, int **res, int n);

void subtract(int **mat1, int **mat2, int **res, int n);

void freeMatrix(int **m, int n);

#endif //CRIPTO_TP_MATRIX_H

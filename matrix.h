#ifndef CRIPTO_TP_MATRIX_H
#define CRIPTO_TP_MATRIX_H

int determinantOfMatrix(int **mat, int N, int n);

//void transpose(int **A, int **B, int n, int k);

int **transpose(int **A, int n, int k);

int **multiply(int **mat1, int **mat2, int n, int m, int k);

//void add(int **mat1, int **mat2, int **res, int n);

void subtract(int **mat1, int **mat2, int **res, int n);

int **inverse(int **m, int n, int inverses[251]);

int **add(int **mat1, int **mat2, int n);

//int **subtract(int **mat1, int **mat2, int n);

void freeMatrix(int **m, int n);

//Kevin's

long* generateVector(int k, int initialValue);

void printVector(int k, long* array);

long** generateMatrixX(int k, int n);

void printMatrix(int k, int n, long** matrix);

long** transposeV2(long** matrix, int n, int k); //TODO transpose MIGRATE TO THIS

long** multiplyV2(long **mat1, long **mat2, int n, int m, int k); //TODO multiply MIGRATE TO THIS

#endif //CRIPTO_TP_MATRIX_H

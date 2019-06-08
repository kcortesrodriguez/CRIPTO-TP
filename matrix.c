//
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrix.h"
#include "random.h"

// Function to get cofactor of mat[p][q] in temp[][]. n is current
// dimension of mat[][]
void getCofactor(int **mat, int **temp, int p, int q, int n) {
    int i = 0, j = 0;

    // Looping for each element of the matrix
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            //  Copying into temporary matrix only those element
            //  which are not in given row and column
            if (row != p && col != q) {
                temp[i][j++] = mat[row][col];

                // Row is filled, so increase row index and
                // reset col index
                if (j == n - 1) {
                    j = 0;
                    i++;
                }
            }
        }
    }
}

/* Recursive function for finding determinant of matrix.
   n is current dimension of mat[][], N is the original dimension of the matrix. */
int determinantOfMatrix(int **mat, int N, int n) {
    int D = 0; // Initialize result

    //  Base case : if matrix contains single element
    if (n == 1)
        return mat[0][0];

    // To store cofactors
    int **temp = (int **) malloc(N * sizeof(int *)); //TODO free
    for (int i = 0; i < N; i++) temp[i] = (int *) malloc(N * sizeof(int));

    int sign = 1;  // To store sign multiplier

    // Iterate for each element of first row
    for (int f = 0; f < n; f++) {
        // Getting Cofactor of mat[0][f]
        getCofactor(mat, temp, 0, f, n);
        D += sign * mat[0][f] * determinantOfMatrix(temp, N, n - 1);

        // terms are to be added with alternate sign
        sign = -sign;
    }

    return D;
}

// This function stores transpose of A[][] in B[][]
void transpose(int **A, int **B, int n, int k) {
    int i, j;
    for (i = 0; i < k; i++)
        for (j = 0; j < n; j++)
            B[i][j] = A[j][i];
}

// This function multiplies mat1[][] and mat2[][],
// and stores the result in res[][]
// n is the rows of mat1
// m is the cols of mat2
// k is the cols of mat1 and rows of mat2
void multiply(int **mat1, int **mat2, int **res, int n, int m, int k) {
    int i, j, h;
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            res[i][j] = 0;
            for (h = 0; h < k; h++)
                res[i][j] += mat1[i][h] * mat2[h][j];
            res[i][j] = res[i][j] % 251;
        }
    }
}

long* generateVector(int k, int initialValue){
    int i;

    long *array;
    array = (long *)malloc(sizeof(long) * k);

    for(i=0; i < k; i++) {
        //TODO: CHECK IF IT HAS TO BE Z 251
        array[i] = ( (long)pow(initialValue, i) ) % 251;
    }

    return array;
}

void printVector(int k, long* array){
    for(int i=0; i < k; i++) {
        printf("%ld ", array[i]);
    }
}

long** generateMatrixX(int k, int n){
    long **temp = (long **) malloc(n * sizeof(long *)); //TODO free
    int* randoms = generateRandoms(n);

    for (int i = 0; i < n; i++) {
        temp[i] = generateVector(k, randoms[i]);
    }

    return temp;
}

void printMatrix(int k, int n, long** matrix){
    for (int i = 0; i < n; i++) {
        printVector(k, matrix[i]);
        printf("\n");
    }
}
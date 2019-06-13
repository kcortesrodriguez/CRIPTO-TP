#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrix.h"
#include "modular.h"
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

                // Row is filled, so increase row index and reset col index
                if (j == n - 1) {
                    j = 0;
                    i++;
                }
            }
        }
    }
}

// Recursive function for finding determinant of matrix.
// n is current dimension of mat[][], N is the original dimension of the matrix.
int determinantOfMatrix(int **mat, int N, int n) {
    int D = 0; // Initialize result

    //  Base case : if matrix contains single element
    if (n == 1)
        return mat[0][0];

    // To store cofactors
    int **temp = (int **) malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) temp[i] = (int *) calloc((size_t) N, sizeof(int));

    int sign = 1;  // To store sign multiplier

    // Iterate for each element of first row
    for (int f = 0; f < n; f++) {
        // Getting Cofactor of mat[0][f]
        getCofactor(mat, temp, 0, f, n);
        D += sign * mat[0][f] * determinantOfMatrix(temp, N, n - 1);

        // terms are to be added with alternate sign
        sign = -sign;
    }

    freeMatrix(temp, n);

    return D;
}

// This function stores transpose of A[][] in B[][]
int **transpose(int **A, int n, int k) {
    int **transposeMatrix = (int **) calloc(n, sizeof(int *)); //TODO free
    for (int i = 0; i < n; i++) {
        transposeMatrix[i] = (int *) calloc(k, sizeof(int));
    }


    for (int i = 0; i < k; i++)
        for (int j = 0; j < n; j++){
            transposeMatrix[i][j] = A[j][i];
        }

    return transposeMatrix;
}

// This function multiplies mat1[][] and mat2[][],
// and stores the result in res[][]
// n is the rows of mat1
// m is the cols of mat2
// k is the cols of mat1 and rows of mat2
int **multiply(int **mat1, int **mat2, int n, int m, int k) {

    int **res = (int **) calloc(n, sizeof(int *)); //TODO free
    for (int i = 0; i < n; i++) {
        res[i] = (int *) calloc(k, sizeof(int));
    }

    int i, j, h;
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            res[i][j] = 0;
            for (h = 0; h < k; h++)
                res[i][j] += mat1[i][h] * mat2[h][j];
            res[i][j] = modulo(res[i][j], 251);
        }
    }

    return res;
}

// Function to perform the inverse operation on the matrix.
int **inverse(int **m, int n, int inverses[251]) {

    int **mInverse = (int **) malloc(n * sizeof(int *)); //TODO free
    for (int i = 0; i < n; i++) mInverse[i] = (int *) calloc((size_t) n * 2, sizeof(int));


    printf("\n");
    printf("m matrix:\n");
    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < n; columns++) {
            printf("  %d", m[row][columns]);
        }
        printf("\n");
    }

    // Create the augmented matrix
    // Add the identity matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 2 * n; j++) {

            if (j < n)
                mInverse[i][j] = m[i][j];

            // Add '1' at the diagonal places
            if (j == (i + n))
                mInverse[i][j] = 1;
        }
    }

    printf("\n");
    printf("Aug plus id matrix:\n");
    for (int row = 0; row < n; row++) {
        for (int columns = 0; columns < 2 * n; columns++) {
            printf("  %d", mInverse[row][columns]);
        }
        printf("\n");
    }

    // Interchange the row of matrix, starting from the last row
    for (int i = n - 1; i > 0; i--) {

        // Swapping the rows
        if (mInverse[i - 1][0] < mInverse[i][0]) {
            int *temp = mInverse[i];
            mInverse[i] = mInverse[i - 1];
            mInverse[i - 1] = temp;
        }
    }

    int temp;

    // Replace a row by sum of itself and a
    // constant multiple of another row of the matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (j != i) {
                temp = modulo(mInverse[j][i] * inverses[mInverse[i][i]], 251);
                for (int k = 0; k < 2 * n; k++) {
                    mInverse[j][k] = modulo(mInverse[j][k] - modulo(mInverse[i][k] * temp, 251), 251);
                }
            }
        }
    }

    // Multiply each row by a nonzero integer.
    // Divide row element by the diagonal element
    for (int i = 0; i < n; i++) {
        temp = mInverse[i][i];
        for (int j = 0; j < 2 * n; j++) {
            mInverse[i][j] = modulo(mInverse[i][j] * inverses[temp], 251);
        }
    }

    return mInverse;
}

int **add(int **mat1, int **mat2, int n) {
    int **res = (int **) malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) res[i] = (int *) calloc((size_t) n, sizeof(int));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            res[i][j] = modulo(mat1[i][j] + mat2[i][j], 251);
        }
    }

    return res;

}

void subtract(int **mat1, int **mat2, int **res, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            res[i][j] = modulo(mat1[i][j] - mat2[i][j], 251);
        }
    }
}

void freeMatrix(int **m, int n) {
    for (int i = 0; i < n; i++) {
        int *currentIntPtr = m[i];
        free(currentIntPtr);
    }
    free(m);
}

//From now on, these functions are from Kevin

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

long** transposeV2(long** matrix, int n, int k){

    long **transposeMatrix = (long **) malloc(n * sizeof(long *)); //TODO free
    for (int i = 0; i < n; i++) {
        transposeMatrix[i] = (long *) malloc(k * sizeof(long));
    }


    for (int i = 0; i < k; i++)
        for (int j = 0; j < n; j++){
            transposeMatrix[j][i] = matrix[i][j];
        }

    return transposeMatrix;

}

long **multiplyV2(long **mat1, long **mat2, int n, int m, int k) {

    long **res = (long **) malloc(n * sizeof(long *)); //TODO free
    for (int i = 0; i < n; i++) {
        res[i] = (long *) malloc(k * sizeof(long));
    }

    int i, j, h;
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            res[i][j] = 0;
            for (h = 0; h < k; h++)
                res[i][j] += mat1[i][h] * mat2[h][j];
            res[i][j] = res[i][j] % 251;
        }
    }

    return res;
}
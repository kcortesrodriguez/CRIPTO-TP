#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrix.h"
#include "modular.h"
#include "random.h"
#include "global.h"
#include "distribution.h"

// Function to get cofactor of mat[p][q] in temp[][]. n is current
// dimension of mat[][]
void getCofactor(long **mat, long **temp, int p, int q, int n) {
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
long determinantOfMatrix(long **mat, int N, int n) {
    long D = 0; // Initialize result

    //  Base case : if matrix contains single element
    if (n == 1)
        return mat[0][0];

    // To store cofactors
    long **temp = (long **) malloc(N * sizeof(long *));
    for (int i = 0; i < N; i++) temp[i] = (long *) calloc((size_t) N, sizeof(long));

    int sign = 1;  // To store sign multiplier

    // Iterate for each element of first row
    for (int f = 0; f < n; f++) {
        // Getting Cofactor of mat[0][f]
        getCofactor(mat, temp, 0, f, n);
        D += sign * mat[0][f] * determinantOfMatrix(temp, N, n - 1);

        // terms are to be added with alternate sign
        sign = -sign;
    }

    freeLongMatrix(temp, N);

    return D;
}

// This function stores transpose of A[][] in B[][]
long **transpose(long **mat, int n, int k) {
    long **transposeMatrix = (long **) malloc(k * sizeof(long *));
    for (int i = 0; i < k; i++) {
        transposeMatrix[i] = (long *) calloc((size_t) n, sizeof(long));
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < n; j++) {
            transposeMatrix[i][j] = mat[j][i];
        }
    }

    return transposeMatrix;
}

// This function multiplies mat1[][] and mat2[][],
// and stores the result in res[][]
// n is the rows of mat1
// m is the cols of mat2
// k is the cols of mat1 and rows of mat2
long **multiply(long **mat1, long **mat2, int n, int m, int k) {
    long **res = (long **) malloc((size_t) n * sizeof(long *));
    for (int i = 0; i < n; i++) {
        res[i] = (long *) calloc((size_t) m, sizeof(long));
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            res[i][j] = 0L;
            for (int h = 0; h < k; h++)
                res[i][j] += mat1[i][h] * mat2[h][j];
            res[i][j] = modulo(res[i][j], 251);
        }
    }

    return res;
}

// Function to perform the inverse operation on the matrix.
long **inverse(long **m, int n, int inverses[251]) {

    long **mInverse = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) mInverse[i] = (long *) calloc((size_t) n * 2, sizeof(long));

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

//    printMatrix(2 * n, n, mInverse, "Aug plus id matrix");

    // Interchange the row of matrix, starting from the last row
    for (int i = n - 1; i > 0; i--) {

        // Swapping the rows
        if (mInverse[i - 1][0] < mInverse[i][0]) {
            long *temp = mInverse[i];
            mInverse[i] = mInverse[i - 1];
            mInverse[i - 1] = temp;
        }
    }

    long temp;

    // Replace a row by sum of itself and a
    // constant multiple of another row of the matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (j != i) {
                temp = modulo(mInverse[j][i] * inverses[mInverse[i][i]], 251);
                for (int k = 0; k < 2 * n; k++) { // TODO: watchout 2 * n vs. n + 1 (kprime)
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

long **add(long **mat1, long **mat2, int n) {
    long **res = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) res[i] = (long *) calloc(n, sizeof(long));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            res[i][j] = modulo(mat1[i][j] + mat2[i][j], 251);
        }
    }

    return res;

}

long **subtract(long **mat1, long **mat2, int n) {
    long **res = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) res[i] = (long *) calloc((size_t) n, sizeof(long));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            res[i][j] = modulo(mat1[i][j] - mat2[i][j], 251);
        }
    }

    return res;
}

void freeLongMatrix(long **m, int n) {
    for (int i = 0; i < n; i++) {
        long *current_ptr = m[i];
        free(current_ptr);
    }
    free(m);
}

void freeCharMatrix(char **m, int n) {
    for (int i = 0; i < n; i++) {
        free(m[i]);
    }
    free(m);
}

void printVector(int k, long *array, char *title) {
    if (!VERBOSE) return;
    printf("\n%s\n", title);
    for (int i = 0; i < k; i++) {
        printf("%ld ", array[i]);
    }
    printf("\n");
}

void printMatrix(int k, int n, long **matrix, char *title) {
    if (!VERBOSE) return;
    printf("\n%s\n", title);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            printf("%ld\t", matrix[i][j]);
        }
        printf("\n");
    }
}

long **concatVecMat(long *vec, long **mat, int n, int m) {
    long **res = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) {
        res[i] = (long *) calloc(((size_t) m + 1), sizeof(long));
    }

    // Fill vec in res
    for (int i = 0; i < n; i++) {
        res[i][0] = vec[i];
    }

    // Fill mat in res
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            res[i][j + 1] = mat[i][j];
        }
    }

    return res;
}

/*
 * mat1 and mat2 must have the same number of rows
 */
long **concatMatMat(long **mat1, long **mat2, int rows, int cols1, int cols2) {
    long **res = (long **) malloc(rows * sizeof(long *));
    for (int i = 0; i < rows; i++) {
        res[i] = (long *) calloc(((size_t) (cols1 + cols2)), sizeof(long));
    }

    // Fill mat1 in res
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols1; j++) {
            res[i][j] = mat1[i][j];
        }
    }

    // Fill mat2 in res
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols2; j++) {
            res[i][j + cols1] = mat2[i][j];
        }
    }

    return res;
}


uint8_t **convertMatrixFromLongToUint8(long **mat, int n, int k) {
    uint8_t **res = (uint8_t **) malloc(n * sizeof(uint8_t *));
    for (int i = 0; i < n; i++) {
        res[i] = (uint8_t *) calloc((size_t) k, sizeof(uint8_t));
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            res[i][j] = (uint8_t) mat[i][j];
        }
    }

    return res;
}

long **convertUint8StreamToLongMatrix(uint8_t *stream, int n, int k) {
    long **res = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) {
        res[i] = (long *) calloc((size_t) k, sizeof(long));
    }
    for (int i = 0; i < n * k; i++) res[i / n][i % k] = stream[i];
    return res;
}

/**
 * Receives an nX(k+1) matrix and returns an nXk matrix by removing the first column
 *
 * @param mat the Sh_t matrix
 * @param n rows on G
 * @param k columns on G
 * @return the G_t matrix
 */
long **deconcatG(long **mat, int n, int k) {
    long **res = (long **) calloc((size_t) n, sizeof(long *)); //TODO free
    for (int i = 0; i < n; i++)
        res[i] = (long *) calloc((size_t) k, sizeof(long));

    for (int j = 0; j < n; ++j) {
        for (int i = 1; i < k + 1; ++i) {
            res[j][i - 1] = mat[j][i];
        }
    }

    return res;
}

/**
 * Receives an nX* matrix and returns a vector with the first column
 *
 * @param mat the Sh_t matrix
 * @param n rows on G
 * @return the G_t matrix
 */
long *deconcatV(long **mat, int n) {
    long *res = (long *) calloc((size_t) n, sizeof(long)); //TODO free

    for (int j = 0; j < n; j++) {
        res[j] = mat[j][0];
    }

    return res;
}


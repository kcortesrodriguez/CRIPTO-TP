#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
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
int determinantOfMatrix(long **mat, int N, int n) {
    int D = 0; // Initialize result

    //  Base case : if matrix contains single element
    if (n == 1)
        return (int) mat[0][0];

    // To store cofactors
    long **temp = (long **) malloc(N * sizeof(long *));
    for (int i = 0; i < N; i++) temp[i] = (long *) calloc((size_t) N, sizeof(long));

    int sign = 1;  // To store sign multiplier

    // Iterate for each element of first row
    for (int f = 0; f < n; f++) {
        // Getting Cofactor of mat[0][f]
        getCofactor(mat, temp, 0, f, n);
        D += modulo(sign * mat[0][f] * determinantOfMatrix(temp, N, n - 1), 251);
        D = (int) modulo(D, 251);
        // terms are to be added with alternate sign
        sign = -sign;
    }

    freeLongMatrix(temp, N);

    return D;
}

// Function to get adjoint of A[N][N] in adj[N][N].
void adjoint(long **A, int N, long **adj) {
    if (N == 1) {
        adj[0][0] = 1;
        return;
    }

    // temp is used to store cofactors of A[][]
    int sign;
    long **temp = (long **) malloc(N * sizeof(long *));
    for (int i = 0; i < N; i++) temp[i] = (long *) calloc((size_t) N, sizeof(long));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            // Get cofactor of A[i][j]
            getCofactor(A, temp, i, j, N);

            // sign of adj[j][i] positive if sum of row
            // and column indexes is even.
            sign = ((i + j) % 2 == 0) ? 1 : -1;

            // Interchanging rows and columns to get the
            // transpose of the cofactor matrix
            adj[j][i] = modulo((sign) * (determinantOfMatrix(temp, N, N - 1)), 251);
        }
    }

    freeLongMatrix(temp, N);
}

// Function to calculate and store inverse, returns false if
// matrix is singular
bool inverseWithAdjoint(long **A, int N, long **inverse, const int *inverses) {
    // Find determinant of A[][]
    int det = determinantOfMatrix(A, N, N);
    if (det == 0) {
        return false;
    }

    // Find adjoint
    long **adj = (long **) malloc(N * sizeof(long *));
    for (int i = 0; i < N; i++) adj[i] = (long *) calloc((size_t) N, sizeof(long));

    adjoint(A, N, adj);

    // Find Inverse using formula "inverse(A) = adj(A)/det(A)"
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            inverse[i][j] = modulo(adj[i][j] * inverses[det], 251);

    freeLongMatrix(adj, N);

    return true;
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
            for (int h = 0; h < k; h++) {
                res[i][j] = modulo(res[i][j], 251) +
                            modulo(mul_mod((uint32_t) mat1[i][h], (uint32_t) mat2[h][j], 251), 251);
            }
            res[i][j] = modulo(res[i][j], 251);
        }
    }

    return res;
}

long **add(long **mat1, long **mat2, int n) {
    long **res = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) res[i] = (long *) calloc((size_t) n, sizeof(long));

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
    long **res = (long **) calloc((size_t) n, sizeof(long *));
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
    long *res = (long *) calloc((size_t) n, sizeof(long));

    for (int j = 0; j < n; j++) {
        res[j] = mat[j][0];
    }

    return res;
}


/**
 * function for exchanging two rows of a double matrix
 */
void swapDouble(double **mat, int row1, int row2, int col) {
    for (int i = 0; i < col; i++) {
        double temp = mat[row1][i];
        mat[row1][i] = mat[row2][i];
        mat[row2][i] = temp;
    }
}

double **cloneLongToDoubleMatrix(long **mat, int n, int k) {
    double **res = (double **) malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        res[i] = (double *) calloc((size_t) k, sizeof(double));
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            res[i][j] = (double) mat[i][j];
        }
    }
    return res;
}

/**
 * function for finding rank of matrix
 */
int rankOfMatrix(long **mat, int R, int C) {
    int rank = C;

    double **aux = cloneLongToDoubleMatrix(mat, R, C);

    for (int row = 0; row < rank; row++) {
        // Before we visit current row 'row', we make 
        // sure that mat[row][0],....mat[row][row-1] 
        // are 0. 

        // Diagonal element is not zero 
        if (aux[row][row]) {
            for (int col = 0; col < R; col++) {
                if (col != row) {
                    // This makes all entries of current 
                    // column as 0 except entry 'mat[row][row]' 
                    double mult = aux[col][row] / aux[row][row];
                    for (int i = 0; i < rank; i++)
                        aux[col][i] -= mult * aux[row][i];
                }
            }
        }

            // Diagonal element is already zero. Two cases 
            // arise: 
            // 1) If there is a row below it with non-zero 
            //    entry, then swap this row with that row 
            //    and process that row 
            // 2) If all elements in current column below 
            //    mat[r][row] are 0, then remvoe this column 
            //    by swapping it with last column and 
            //    reducing number of columns by 1. 
        else {
            bool reduce = true;

            /* Find the non-zero element in current 
                column  */
            for (int i = row + 1; i < R; i++) {
                // Swap the row with non-zero element 
                // with this row. 
                if (aux[i][row]) {
                    swapDouble(aux, row, i, rank);
                    reduce = false;
                    break;
                }
            }

            // If we did not find any row with non-zero 
            // element in current columnm, then all 
            // values in this column are 0. 
            if (reduce) {
                // Reduce number of columns 
                rank--;

                // Copy the last column here 
                for (int i = 0; i < R; i++)
                    aux[i][row] = aux[i][rank];
            }

            // Process this row again 
            row--;
        }

        // Uncomment these lines to see intermediate results 
        // display(mat, R, C); 
        // printf("\n"); 
    }

    for (int i = 0; i < R; i++) {
        double *current_ptr = aux[i];
        free(current_ptr);
    }
    free(aux);

    return rank;
}

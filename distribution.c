#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <libgen.h>
#include "random.h"
#include "distribution.h"
#include "matrix.h"
#include "modular.h"
#include "global.h"
#include "io.h"

// rank k
// det != 0
// proj(A) y S - proj(A) no debe tener valores mayores que 251
long **matA(int n, int k) {
    long **A = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) A[i] = (long *) malloc(k * sizeof(long));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            A[i][j] = modulo(urandom(), 251); // 251 because A must have values in [0, 251)
        }
    }

//    printMatrix(k, n, A, "A matrix");

    return A;
}

long **projectionSd(long **A, int n, int k, int inverses[251]) {

    long **At = transpose(A, n, k);
//    printMatrix(n, k, At, "At matrix");

    long **AtA = multiply(At, A, k, k, n);
//    printMatrix(k, k, AtA, "AtA matrix");

    long det = determinantOfMatrix(AtA, k, k);

    if (det != 0) {
//        if (VERBOSE) printf("\n%s\n", "Inverse exists!");

        long **AugmentedAtaInverse = inverse(AtA, k, inverses);
//        printMatrix(k * 2, k, AugmentedAtaInverse, "AugmentedAtaInverse matrix");

        long **AtAInverse = (long **) malloc(k * sizeof(long *));
        for (int i = 0; i < k; i++) AtAInverse[i] = (long *) calloc((size_t) k, sizeof(long));

        // Fill AtAInverse
        for (int row = 0; row < k; row++) {
            for (int columns = 0; columns < k; columns++) {
                AtAInverse[row][columns] = AugmentedAtaInverse[row][columns + k];
            }
        }

//        printMatrix(k, k, AtAInverse, "AtAInverse matrix");

        // A * (At * A)'
        long **AxInverse = multiply(A, AtAInverse, n, k, k);
//        printMatrix(k, n, AxInverse, "AxInverse matrix");

        // (A * (At * A)') * At
        long **proj = multiply(AxInverse, At, n, n, k);

        freeLongMatrix(AugmentedAtaInverse, k);
        freeLongMatrix(AtA, k);
        freeLongMatrix(AtAInverse, k);
        freeLongMatrix(AxInverse, n);
        freeLongMatrix(At, k);

        return proj;
    } else {
        if (VERBOSE) printf("%s", "Inverse does not exist!");

        freeLongMatrix(AtA, k);
        freeLongMatrix(At, k);

        return NULL;
    }
}

long **remainderR(long **secretS, long **projectionSd, int n) {
    return subtract(secretS, projectionSd, n);
}

long **remainderRw(long **watermarkW, long **projectionSd, int n) {
    return subtract(watermarkW, projectionSd, n);
}

/*
 * R is the remainder matrix.
 * c is the coefficients array (from 1 to n).
 * n is the max number of participants.
 * k is the min number of participants.
 * t is the current participant index.
 */
long *g_i_j(long **R, int initial_column, int cj, int n, int k) {
    long *res = (long *) calloc((size_t) n, sizeof(long));

    for (int row = 0; row < n; row++) {
        for (int column = initial_column; column < k + initial_column; column++) {
            res[row] += modulo((long) (R[row][column] * pow((double) cj, (double) (column - initial_column))), 251);
        }
        res[row] = modulo(res[row], 251);
    }

    return res;
}

/*
 * R is the remainder matrix.
 * n is the max number of participants.
 * k is the min number of participants.
 * t is the current participant index.
 */
long **matG_t(long **R, int n, int k, int t) {
    int G_t_cols = (int) ceil((double) n / k); // TODO: code method for create matrix !
    long **res = (long **) calloc((size_t) G_t_cols, sizeof(long *));

    for (int i = 0; i < G_t_cols; i++) {
        res[i] = g_i_j(R, i * k, t, n, k);
    }

    long **resT = transpose(res, G_t_cols, n);

    freeLongMatrix(res, G_t_cols);

    return resT;
}

/*
 * R is the remainder matrix.
 * n is the max number of participants.
 * k is the min number of participants.
 */
long ***matG(long **R, int n, int k) {
    long ***matG = (long ***) malloc(n * sizeof(long **));

    for (int t = 0; t < n; t++) {
        matG[t] = matG_t(R, n, k, t + 1);
    }

    return matG;
}

long **matV(long **A, long **X, int n, int k) {
    long **matrixMultiply;
    matrixMultiply = multiply(A, X, n, n, k);
    return matrixMultiply;
}

long ***matSh(long ***G, long **V, int n, int k) {
    long ***Sh = (long ***) malloc(n * sizeof(long **));

    for (int i = 0; i < n; i++) {
        Sh[i] = concatVecMat(transpose(V, n, n)[i], G[i], n, (int) (ceil((double) n / k)));
    }

    return Sh;
}

long *generateVector(int k, int initialValue) {
    long *array = (long *) malloc(sizeof(long) * k);

    for (int i = 0; i < k; i++) {
        array[i] = modulo((long) pow((double) initialValue, (double) i), 251);
    }

    return array;
}

/*
 * Generate matrix X.
 * N vector of dimensions k x 1, linearly independent.
 * k columns
 * n rows
 */
long **matX(int k, int n) {
    long **temp = (long **) malloc(n * sizeof(long *));
    int *randoms = generateRandoms(n);

    for (int i = 0; i < n; i++) {
        temp[i] = generateVector(k, randoms[i]);
    }

    free(randoms);

    long **res = transpose(temp, n, k);

    freeLongMatrix(temp, n);

    return res;
}

void initialize_shadow_bmp_files(int n, char **shadow_files, BITMAP_FILE **shadow_bmps, char *output_lsb_dir) {

    // Traverse shadow bmp files
    for (int t = 0; t < n; t++) {

        // Get bmp path
        char *shadow_bmp_path = shadow_files[t];

        // Load bmp
        BITMAP_FILE *shadow_bmp = load_BMP(shadow_bmp_path);

        // Set new name for bmp
        char lsb_shadow_bmp_name[MAX_PATH];
        memset(lsb_shadow_bmp_name, 0, strlen(lsb_shadow_bmp_name));
        strcat(lsb_shadow_bmp_name, output_lsb_dir);
        strcat(lsb_shadow_bmp_name, basename(shadow_bmp_path));
        memset(shadow_bmp->fname, 0, sizeof shadow_bmp->fname);
        strcpy(shadow_bmp->fname, lsb_shadow_bmp_name);

        // Save order number at reserved byte
        shadow_bmp->header.file.res1 = (unsigned short) (t + 1);

        // Set altered shadow bmp file
        shadow_bmps[t] = shadow_bmp;
    }
}

void distribute(int n,
                int k,
                int *inverses,
                BITMAP_FILE *secret_bmp,
                BITMAP_FILE *watermark_bmp,
                BITMAP_FILE *rw_bmp,
                int *shadow_bmps_index,
                BITMAP_FILE **shadow_bmps) {

    // Current Rw byte index
    int current_rw_byte_index = 0;

    // Traverse 8 bpp secret image n x n bytes at a time
    int s_matrices = secret_bmp->header.info.image_size / (n * n);
    for (int i = 0; i < s_matrices; i++) {

        // Matrix S
        // Convert secret stream to n x n matrix
        long **S = convertUint8StreamToLongMatrix(secret_bmp->data + (i * n * n), n, n);
//        printMatrix(n, n, S, "S matrix:");

        // Matrix A
        long **A = matA(n, k);

        // Matrix Sd
        long **Sd = projectionSd(A, n, k, inverses);
        printMatrix(n, n, Sd, "Sd matrix");

        // Matrix R
        long **R = remainderR(S, Sd, n);
//        printMatrix(n, n, R, "R matrix"); //TODO write in report that R matrix from paper is wrong.

        // Matrix W
        // Convert watermark stream to n x n matrix
        long **W = convertUint8StreamToLongMatrix(watermark_bmp->data + (i * n * n), n, n);

        // Matrix Rw
        long **Rw = remainderRw(W, Sd, n);
//        printMatrix(n, n, Rw, "Rw matrix:");

        // Fill Rw
        for (int p = 0; p < n; p++) {
            for (int q = 0; q < n; q++) {
                // Set bit on rw_bmp->data
                rw_bmp->data[current_rw_byte_index] = (uint8_t) Rw[p][q];
                current_rw_byte_index++;
            }
        }

        // Matrix X
        long **X = matX(k, n);
//        printMatrix(n, k, X, "X matrix");

        // Matrix V
        long **V = matV(A, X, n, k);
        printMatrix(n, n, V, "V matrix");

        // Matrix G
        long ***G = matG(R, n, k);
        for (int t = 0; t < n && VERBOSE; t++) {
//            printMatrix((int) ceil((double) n / k), n, G[t], "G_ matrix");
        }

        // Matrix Sh
        long ***Sh = matSh(G, V, n, k);
        uint8_t ***uint8_Sh = (uint8_t ***) malloc(n * sizeof(uint8_t **));
        for (int t = 0; t < n; t++) {
//            printMatrix((int) (ceil((double) n / k) + 1), n, Sh[t], "Sh_ matrix");
            uint8_Sh[t] = convertMatrixFromLongToUint8(Sh[t], n, (int) (ceil((double) n / k) + 1));
        }

        // Traverse participants
        for (int t = 0; t < n; t++) {

            // Get current shadow byte index
            int current_shadow_byte_index = shadow_bmps_index[t];

            // Traverse each byte of shadow image
            for (int a = 0; a < n; a++) {
                for (int b = 0; b < (int) (ceil((double) n / k) + 1); b++) {
                    uint8_t current_byte = uint8_Sh[t][a][b];

                    // Traverse bits of current byte
                    for (int l = 0; l < 8; l++) {
                        uint8_t one_or_zero = (uint8_t) ((current_byte >> l) & 0x01);

                        // Set bit on shadow
                        uint8_t current_movie_byte = (uint8_t) shadow_bmps[t]->data[current_shadow_byte_index];
                        shadow_bmps[t]->data[current_shadow_byte_index] =
                                (uint8_t) (current_movie_byte & ~1) | one_or_zero;
                        current_shadow_byte_index++;
                    }
                }
            }

            shadow_bmps_index[t] = current_shadow_byte_index;
        }

        // Destroy resources
        freeLongMatrix(S, n);
        freeLongMatrix(A, n);
        freeLongMatrix(Sd, n);
        freeLongMatrix(R, n);
        freeLongMatrix(W, n);
        freeLongMatrix(Rw, n);
        freeLongMatrix(X, k);
        freeLongMatrix(V, n);
        for (int t = 0; t < n; t++) {
            freeLongMatrix(G[t], n);
        }
        free(G);
        for (int t = 0; t < n; t++) {
            freeLongMatrix(Sh[t], n);
        }
        free(Sh);
        for (int t = 0; t < n; t++) {
            freeCharMatrix((char **) uint8_Sh[t], n);
        }
        free(uint8_Sh);
    }
}

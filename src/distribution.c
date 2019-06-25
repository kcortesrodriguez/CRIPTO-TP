#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <libgen.h>
#include <err.h>
#include "random.h"
#include "distribution.h"
#include "matrix.h"
#include "modular.h"
#include "global.h"
#include "io.h"
#include "recovery.h"

// rank k
// det != 0
// proj(A) y S - proj(A) no debe tener valores mayores que 251
long **matA(int n, int k) {
    long **A = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++) A[i] = (long *) calloc((size_t) k, sizeof(long));

    int rank_of_A;
    int rank_of_AtA;
    do {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < k; j++) {
                A[i][j] = (uint32_t) safe_next_char(); // 251 because A must have values in [0, 251)
            }
        }

        rank_of_A = rankOfMatrix(A, n, k);

        long **At = transpose(A, n, k);
        long **AtA = multiply(At, A, k, k, n);
        rank_of_AtA = rankOfMatrix(AtA, k, k);

        freeLongMatrix(At, k);
        freeLongMatrix(AtA, k);

    } while (rank_of_A != k || rank_of_AtA != k);

    return A;
}

long **projectionSd(long **A, int n, int k, int inverses[251]) {

    long **At = transpose(A, n, k);

    long **AtA = multiply(At, A, k, k, n);

    long det = determinantOfMatrix(AtA, k, k);

    if (det != 0) {

        long **AugmentedAtaInverse = (long **) malloc(k * sizeof(long *));
        for (int i = 0; i < k; i++) AugmentedAtaInverse[i] = (long *) calloc((size_t) k, sizeof(long));
        inverseWithAdjoint(AtA, k, AugmentedAtaInverse, inverses);

        // A * (At * A)'
        long **AxInverse = multiply(A, AugmentedAtaInverse, n, k, k);

        // (A * (At * A)') * At
        long **proj = multiply(AxInverse, At, n, n, k);

        freeLongMatrix(AugmentedAtaInverse, k);
        freeLongMatrix(AtA, k);
        freeLongMatrix(AxInverse, n);
        freeLongMatrix(At, k);

        return proj;
    } else {
        if (VERBOSE) printf("%s\n", "Inverse does not exist for Sd projection! Retrying...");

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
    int G_t_cols = (int) ceil((double) n / k);
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

    long **Vt = transpose(V, n, n);

    for (int i = 0; i < n; i++) {
        Sh[i] = concatVecMat(Vt[i], G[i], n, (int) (ceil((double) n / k)));
    }

    freeLongMatrix(Vt, n);

    return Sh;
}

uint32_t int_pow(uint32_t x, uint32_t y) {
    // X^Y
    if (y == 0) {
        return 1;
    }
    uint32_t ret = x;
    for (int i = 0; i < y - 1; i++) {
        ret *= x;
    }
    return ret;
}

long *generateVector(int k, int initialValue) {
    long *array = (long *) malloc(sizeof(long) * k);

    for (int i = 0; i < k; i++) {
        array[i] = modulo(int_pow((uint32_t) initialValue, (uint32_t) i), 251);
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
    uint8_t *randoms = generateRandoms(n);

    for (int i = 0; i < n; i++) {
        temp[i] = generateVector(k, randoms[i]);
    }

    free(randoms);

    long **res = transpose(temp, n, k);

    freeLongMatrix(temp, n);

    return res;
}

void initialize_shadow_bmp_files(int n,
                                 char **shadow_files,
                                 BITMAP_FILE **shadow_bmps,
                                 char *output_lsb_dir,
                                 unsigned int secret_width,
                                 unsigned int secret_height) {

    // Traverse shadow bmp files
    for (int t = 0; t < n; t++) {

        // Get bmp path
        char *shadow_bmp_path = shadow_files[t];

        // Load bmp
        BITMAP_FILE *shadow_bmp = load_BMP(shadow_bmp_path);

        // If image size is not the same as secret's, abort
        if (shadow_bmp->header.info.width != secret_width
            || shadow_bmp->header.info.height != secret_height) {
            errx(EXIT_FAILURE, "A share bmp width or height does not equal secret's.");
        }

        // Set new name for bmp
        char lsb_shadow_bmp_name[MAX_PATH];
        memset(lsb_shadow_bmp_name, 0, strlen(lsb_shadow_bmp_name));
        strcat(lsb_shadow_bmp_name, output_lsb_dir);
        strcat(lsb_shadow_bmp_name, basename(shadow_bmp_path));
        memset(shadow_bmp->fname, 0, sizeof shadow_bmp->fname);
        strcpy(shadow_bmp->fname, lsb_shadow_bmp_name);

        // Save order number at reserved byte
        shadow_bmp->header.file.res1 = (unsigned short) t;

        // Set altered shadow bmp file
        shadow_bmps[t] = shadow_bmp;
    }
}

static void run(int n,
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
        truncateBytesTo250(S, n);

        // Matrix A
        long **A;

        // Matrix Sd
        long **Sd;
        do {
            A = matA(n, k);

            Sd = projectionSd(A, n, k, inverses);

            if (Sd == NULL)
                freeLongMatrix(A, n);
        } while (Sd == NULL);

        // Matrix R
        long **R = remainderR(S, Sd, n);

        // Matrix W
        // Convert watermark stream to n x n matrix
        long **W = convertUint8StreamToLongMatrix(watermark_bmp->data + (i * n * n), n, n);

        // Matrix Rw
        long **Rw = remainderRw(W, Sd, n);

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

        // Matrix V
        long **V = matV(A, X, n, k);

        // Matrix G
        long ***G = matG(R, n, k);

        // Matrix Sh
        long ***Sh = matSh(G, V, n, k);
        uint8_t ***uint8_Sh = (uint8_t ***) malloc(n * sizeof(uint8_t **));
        for (int t = 0; t < n; t++) {
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

                    // Revert so as to read write left to right (as in exercise sheet)
                    current_byte = reverse(current_byte);

                    if (k == 4 && n == 8) {
                        // Traverse bits of current byte
                        for (int l = 0; l < 8; l++) {
                            uint8_t one_or_zero = (uint8_t) ((unsigned) (current_byte >> (unsigned) l) & 1u);

                            // Set bit on shadow
                            shadow_bmps[t]->data[current_shadow_byte_index] = (unsigned char) (
                                    (shadow_bmps[t]->data[current_shadow_byte_index] & ~1u) | (one_or_zero & 1u));

                            current_shadow_byte_index++;
                        }
                    } else if (k == 2 && n == 4) {
                        // Traverse bits of current byte
                        for (int l = 0; l < 8; l = l + 2) {
                            uint8_t first_one_or_zero = (uint8_t) (((unsigned) current_byte >> (unsigned) l) & 1u);
                            uint8_t second_one_or_zero = (uint8_t) (((unsigned) current_byte >> (unsigned) (l + 1)) &
                                                                    1u);

                            uint8_t last_two_bits = (uint8_t) ((unsigned) first_one_or_zero << 1u | second_one_or_zero);

                            // Set bit on shadow
                            uint8_t current_movie_byte = (uint8_t) shadow_bmps[t]->data[current_shadow_byte_index];
                            shadow_bmps[t]->data[current_shadow_byte_index] =
                                    (uint8_t) (current_movie_byte & ~3u) | last_two_bits;

                            current_shadow_byte_index++;
                        }

                    } else {
                        errx(EXIT_FAILURE, "k and n not (2,4) or (4,8) for LSB.");
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

void distribute(int n,
                int k,
                int *inverses,
                char *secretImage,
                char *watermarkImage,
                char *output_dir,
                char *shadowDirectory) {

    // Create output/lsb directory
    char output_lsb_dir[MAX_PATH];
    memset(output_lsb_dir, 0, MAX_PATH);
    strcat(output_lsb_dir, output_dir);
    strcat(output_lsb_dir, "lsb/");
    DIR *output_lsb_dir_struct = createDirectory(output_lsb_dir);

    // Shadow bmp array
    BITMAP_FILE *shadow_bmps[n];

    // Get shadow files at directory
    char **shadow_files = get_shadow_files(shadowDirectory, n);

    // Initialize shadow bmp index array
    int shadow_bmps_index[n];
    memset(shadow_bmps_index, 0, n * sizeof(int));

    // Load secret bmp
    BITMAP_FILE *secret_bmp = load_BMP(secretImage);

    // Initialize shadow bmp files
    initialize_shadow_bmp_files(n,
                                shadow_files,
                                shadow_bmps,
                                output_lsb_dir,
                                secret_bmp->header.info.width,
                                secret_bmp->header.info.height);

    // Load watermark bmp
    BITMAP_FILE *w_bmp = load_BMP(watermarkImage);

    // Create Rw bmp
    char rw_bmp_name[MAX_PATH];
    memset(rw_bmp_name, 0, strlen(rw_bmp_name));
    strcat(rw_bmp_name, output_dir);
    strcat(rw_bmp_name, "RW.bmp");
    BITMAP_FILE *rw_bmp = create_BMP(rw_bmp_name, w_bmp->header.info.width, w_bmp->header.info.height, 8);

    // Distribute secret in shadows and Rw
    run(n, k, inverses, secret_bmp, w_bmp, rw_bmp, shadow_bmps_index, shadow_bmps);

    // Save shadow bmp
    for (int t = 0; t < n; t++) {
        write_BMP(shadow_bmps[t]);
    }

    // Save Rw.bmp
    write_BMP(rw_bmp);

    // Destroy resources
    destroy_BMP(rw_bmp);
    destroy_BMP(w_bmp);
    destroy_BMP(secret_bmp);
    freeCharMatrix(shadow_files, n); // Free for distribution
    for (int t = 0; t < n; t++) {
        destroy_BMP(shadow_bmps[t]);
    }
    closedir(output_lsb_dir_struct);
}

void truncateBytesTo250(long **S, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (((uint8_t) S[i][j]) > 250) {
                S[i][j] = 250;
            }
        }
    }
}

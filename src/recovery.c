#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <err.h>
#include "modular.h"
#include "global.h"
#include "recovery.h"
#include "io.h"
#include "matrix.h"
#include "distribution.h"

uint8_t reverse(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

long *gaussJordan(int n, long **matrix, int inverses[251]) {
    // Genero una copia de la matriz matrix
    long **a = (long **) malloc(n * sizeof(long *));
    for (int i = 0; i < n; i++)
        a[i] = (long *) calloc(((size_t) n + 1), sizeof(long));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < (n + 1); j++) {
            a[i][j] = matrix[i][j];
        }
    }

    int kprime = n + 1;
    long t;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j) {
                t = modulo(a[j][i] * inverses[a[i][i]], 251);
                for (int k = 0; k < kprime; k++)
                    a[j][k] = modulo(a[j][k] - modulo(a[i][k] * t, 251), 251);
            }
        }
    }

    // Hago un vector para devolver los valores en long!
    long *solutionVector = calloc((size_t) n, sizeof(long));
    for (int i = 0; i < n; i++) {
        solutionVector[i] = modulo(a[i][n] * inverses[a[i][i]], 251);
    }

    freeLongMatrix(a, n);

    return solutionVector;
}

/**
 * Constructs the matrix of coefficients
 * elevated to the power from 0 to k-1, for each coefficient
 * Each coefficient is of value (i + 1) at the second loop.
 * @param j rows
 * @param k columns, and limit power
 * @return
 */
long **matrixCj(int j, int k, BITMAP_FILE **shadow_bmps_recovery) {
    long **a = (long **) malloc(j * sizeof(long *));
    for (int i = 0; i < j; i++)
        a[i] = (long *) calloc((size_t) k, sizeof(long));

    for (int i = 0; i < j; i++) {
        for (int m = 0; m < k; m++) {
            a[i][m] = modulo((long) pow((double) (shadow_bmps_recovery[i]->header.file.res1 + 1), (double) m), 251);
        }
    }

    return a;
}

//devuelve una matriz de 2 x 1. totalGs es el equivalente al j de matrixCj, que es k
// allGs tiene k matrices G, aunque se generen n en distribution
long **resultG(int x, int y, long ***allGs, int totalGs) {
    long **returnMatrix = (long **) malloc(totalGs * sizeof(long *));
    for (int i = 0; i < totalGs; i++)
        returnMatrix[i] = (long *) calloc(1, sizeof(long));

    for (int i = 0; i < totalGs; i++) {
        long **G_i = allGs[i];
        returnMatrix[i][0] = G_i[x][y];
    }

    return returnMatrix;
}

static void run(int n,
                int k,
                int *inverses,
                BITMAP_FILE **shadow_bmps_recovery,
                BITMAP_FILE *rw_bmp,
                BITMAP_FILE *recovered_secret_bmp,
                BITMAP_FILE *recovered_wm_bmp) {

    // Initialize Sh vector of k matrices Shj
    long ***matSh = (long ***) malloc(k * sizeof(long **));

    // Traverse 8 bpp shadow image 24 x 8 bytes at a time for (4,8) or 12 x 8 for (2,4)
    int Gj_cols = (int) ceil((double) n / k);
    int Shj_cols = Gj_cols + 1;
    int Sh_size = n * Shj_cols; // size in number of bytes of matrix; n rows, 1 is for vector V
    int divisor = Sh_size * n; // n bytes from preSh are need for each byte of Sh_i
    int divisor_bytes_segments = shadow_bmps_recovery[0]->header.info.image_size / divisor;

    // Current recovered secret bmp byte index
    int current_recovered_secret_byte_index = 0;

    // Current recovered watermark bmp byte index
    int current_recovered_wp_byte_index = 0;

    // Iterate for divisor bytes segments
    for (int segment_of_divisor_bytes = 0;
         segment_of_divisor_bytes < divisor_bytes_segments; segment_of_divisor_bytes++) {

        // Initialize G vector of k matrices Gj
        long ***matG = (long ***) malloc(k * sizeof(long **));

        // Initialize B vector of k vectors vj
        long **matB = (long **) malloc(k * sizeof(long *));

        // Iterate for each shadow bmp file
        for (int shadow_bmp_index = 0; shadow_bmp_index < k; shadow_bmp_index++) {

            // Initialize Shj
            long **res = (long **) calloc((size_t) n, sizeof(long *));
            for (int i = 0; i < n; i++)
                res[i] = (long *) calloc((size_t) Shj_cols, sizeof(long));

            // Save Shj at matSh
            matSh[shadow_bmp_index] = res;

            int row = 0;
            int col = 0;

            // Iterate the bytes of the segment of this shadow bmp
            for (int b = 0; b < divisor; b++) {
                uint8_t curr_shadow_lsb_byte = (uint8_t) (shadow_bmps_recovery[shadow_bmp_index]->data[b +
                                                                                                       (segment_of_divisor_bytes *
                                                                                                        divisor)]);

                if (k == 4 && n == 8) {
                    uint8_t one_or_zero = (uint8_t) (curr_shadow_lsb_byte & 1u);

                    matSh[shadow_bmp_index][row][col] = (uint8_t) ((uint8_t) (matSh[shadow_bmp_index][row][col] * 2) |
                                                                   one_or_zero);
                } else if (k == 2 && n == 4) {
                    uint8_t first_one_or_zero = (uint8_t) (curr_shadow_lsb_byte & 1u);
                    uint8_t second_one_or_zero = (uint8_t) (curr_shadow_lsb_byte & 2u);

                    uint8_t last_two_bits = (uint8_t) (first_one_or_zero | second_one_or_zero);

                    matSh[shadow_bmp_index][row][col] = (uint8_t) ((uint8_t) (matSh[shadow_bmp_index][row][col] * 4) |
                                                                   last_two_bits);
                }
                if ((b + 1) % n == 0) {
                    // Advance to next Shj col
                    col++;
                    if ((col % Shj_cols) == 0) {
                        col = 0;
                        row++; // Advance to next Shj row
                    }
                    if (row == n) // Advance to next Shj
                        break;
                }
            }

            // Por cada Sh desconcatenamos V_t y G_t
            long **recoveredG = deconcatG(matSh[shadow_bmp_index], n, Gj_cols);
            long *recoveredV = deconcatV(matSh[shadow_bmp_index], n);

            matG[shadow_bmp_index] = recoveredG;
            matB[shadow_bmp_index] = recoveredV;

            freeLongMatrix(matSh[shadow_bmp_index], n);
        }

        // Matrix with 1s at first column and cjs at second column
        long **matCj = matrixCj(k, k, shadow_bmps_recovery);

        // Initialize recovered R
        long **recoveredR = (long **) malloc(n * sizeof(long *));
        for (int i = 0; i < n; i++)
            recoveredR[i] = (long *) calloc((size_t) n, sizeof(long));

        // Traverse all positions of G matrices
        for (int x = 0; x < n; x++) {
            for (int y = 0; y < (int) ceil((double) n / k); y++) {
                // Get current G for x and y
                long **Gxy = resultG(x, y, matG, k);

                // Calculate solution vector
                long **concatCjGs = concatMatMat(matCj, Gxy, k, k, 1);

                long *solutionVector = gaussJordan(k, concatCjGs, inverses);

                // Save solution vector at recovered R
                if (k == 4 && n == 8) {
                    recoveredR[x][0 + k * y] = solutionVector[0];
                    recoveredR[x][1 + k * y] = solutionVector[1];
                    recoveredR[x][2 + k * y] = solutionVector[2];
                    recoveredR[x][3 + k * y] = solutionVector[3];
                } else if (k == 2 && n == 4) {
                    recoveredR[x][0 + k * y] = solutionVector[0];
                    recoveredR[x][1 + k * y] = solutionVector[1];
                }

                // Destroy resources
                free(solutionVector);
                freeLongMatrix(concatCjGs, k);
                freeLongMatrix(Gxy, k);
            }
        }

        // Matrix Recovered V
        long **matBt = transpose(matB, k, n);

        // Matrix Recovered Sd
        long **Sd = projectionSd(matBt, n, k, inverses);

        // Matrix Recovered S
        long **recoveredS = add(Sd, recoveredR, n);

        long **currentRw = convertUint8StreamToLongMatrix(rw_bmp->data + (current_recovered_wp_byte_index * n * n), n,
                                                          n);
        current_recovered_wp_byte_index++;

        // Matrix Recovered W
        long **recoveredW = add(Sd, currentRw, n);

        // Fill recovered S to bmp
        for (int p = 0; p < n; p++) {
            for (int q = 0; q < n; q++) {
                // Set bit on rw_bmp->data
                recovered_secret_bmp->data[current_recovered_secret_byte_index] = (uint8_t) recoveredS[p][q];

                // Save recovered W
                recovered_wm_bmp->data[current_recovered_secret_byte_index] = (uint8_t) recoveredW[p][q];

                // Advance current byte index, used for both
                current_recovered_secret_byte_index++;
            }
        }

        // Destroy resources
        freeLongMatrix(Sd, n);
        freeLongMatrix(recoveredS, n);
        freeLongMatrix(currentRw, n);
        freeLongMatrix(recoveredW, n);
        freeLongMatrix(matB, k);
        freeLongMatrix(recoveredR, n);
        for (int t = 0; t < k; t++) {
            freeLongMatrix(matG[t], n);
        }
        free(matG);
        freeLongMatrix(matBt, n);
        freeLongMatrix(matCj, k);
    }
    free(matSh);
}

void bubbleSortBitmapFiles(BITMAP_FILE **arr, int n) {
    // Base case
    if (n == 1)
        return;

    // One pass of bubble sort. After
    // this pass, the largest element
    // is moved (or bubbled) to end.
    for (int i = 0; i < n - 1; i++)
        if (arr[i]->header.file.res1 > arr[i + 1]->header.file.res1) {
            BITMAP_FILE *aux = arr[i];
            arr[i] = arr[i + 1];
            arr[i + 1] = aux;
        }

    // Largest element is fixed,
    // recur for remaining array
    bubbleSortBitmapFiles(arr, n - 1);
}

void recover(int n,
             int k,
             int *inverses,
             char retrievedImage[260],
             char watermarkTransformationImage[260],
             char output_dir[260],
             char shadowDirectory[260]) {

    // Shadow bmp recovery array
    BITMAP_FILE *n_shadow_bmps_recovery[k];

    // Get shadow bmp files at output directory
    char **shadow_bmp_output_files = get_shadow_files(shadowDirectory, k);

    // Recover shadow bmps
    for (int t = 0; t < k; t++) {
        // Load shadow bmp modified at distribution by LSB
        n_shadow_bmps_recovery[t] = load_BMP(shadow_bmp_output_files[t]);

        // If image size is not the same as secret's, abort
        if (n_shadow_bmps_recovery[t]->header.info.width != n_shadow_bmps_recovery[0]->header.info.width
            || n_shadow_bmps_recovery[t]->header.info.height != n_shadow_bmps_recovery[0]->header.info.height) {
            errx(EXIT_FAILURE, "A share bmp width or height does not equal other bmps.");
        }
    }

    // Sort shadow bmps by order number at reserved byte
    bubbleSortBitmapFiles(n_shadow_bmps_recovery, k);

    // Shadow bmp recovery array of only 4
    BITMAP_FILE *shadow_bmps_recovery[k];
    for (int t = 0; t < k; t++) {
        shadow_bmps_recovery[t] = n_shadow_bmps_recovery[t];
        printf("Loaded BMP with order %d and path %s.\n", shadow_bmps_recovery[t]->header.file.res1,
               shadow_bmps_recovery[t]->fname);
    }

    // Load Rw bmp
    BITMAP_FILE *rw_bmp = load_BMP(watermarkTransformationImage);

    // Create Recovered Watermark bmp
    char recovered_wm_bmp_name[MAX_PATH];
    memset(recovered_wm_bmp_name, 0, strlen(recovered_wm_bmp_name));
    strcat(recovered_wm_bmp_name, output_dir);
    strcat(recovered_wm_bmp_name, "Recovered_Watermark.bmp");
    BITMAP_FILE *recovered_wm_bmp = create_BMP(recovered_wm_bmp_name, rw_bmp->header.info.width,
                                               rw_bmp->header.info.height, 8);

    // Create Recovered Secret bmp
    char recovered_secret_bmp_name[MAX_PATH];
    memset(recovered_secret_bmp_name, 0, strlen(recovered_secret_bmp_name));
    strcat(recovered_secret_bmp_name, retrievedImage);
    BITMAP_FILE *recovered_secret_bmp = create_BMP(recovered_secret_bmp_name, rw_bmp->header.info.width,
                                                   rw_bmp->header.info.height, 8);

    // Recover secret in shadows and watermark
    run(n, k, inverses, shadow_bmps_recovery, rw_bmp, recovered_secret_bmp, recovered_wm_bmp);

    // Save Recovered_Secret.bmp
    write_BMP(recovered_secret_bmp);

    // Save Recovered_Watermark.bmp
    write_BMP(recovered_wm_bmp);

    // Destroy resources
    freeCharMatrix(shadow_bmp_output_files, k);
    for (int t = 0; t < k; t++) {
        destroy_BMP(n_shadow_bmps_recovery[t]);
    }
    destroy_BMP(rw_bmp);
    destroy_BMP(recovered_wm_bmp);
    destroy_BMP(recovered_secret_bmp);
}

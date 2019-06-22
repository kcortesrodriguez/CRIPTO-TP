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

    //Genero una copia de la matriz matrix, pero en floats para operar sobre esta
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

    //Hago un vector para devolver los valores en long!
    long *solutionVector = calloc((size_t) n, sizeof(long));

    for (int i = 0; i < n; i++) {
        solutionVector[i] = modulo(a[i][n] * inverses[a[i][i]], 251);
    }

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
long **matrixCj(int j, int k) {
    long **a = (long **) malloc(j * sizeof(long *));
    for (int i = 0; i < j; i++)
        a[i] = (long *) calloc((size_t) k, sizeof(long));

    for (int i = 0; i < j; i++) {
        for (int m = 0; m < k; m++) {
            a[i][m] = modulo((long) pow((double) (i + 1), (double) m), 251);
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
                BITMAP_FILE *recovered_wm_bmp,
                BITMAP_FILE *secret_bmp) {

    // Initialize Sh vector of k matrices Shj
    long ***matSh = (long ***) malloc(k * sizeof(long **)); //TODO: free

    // Recover shadow bmps
    for (int t = 0; t < k; t++) {
        // Initialize Shj
        long **res = (long **) calloc((size_t) n, sizeof(long *)); //TODO free
        for (int i = 0; i < n; i++)
            res[i] = (long *) calloc((size_t) (ceil((double) n / k) + 1), sizeof(long));

        // Save Shj at matSh
        matSh[t] = res;
    }

    // Traverse 8 bpp shadow image 24 x 8 bytes at a time for (4,8) or 12 x 8 for (2,4)
    int Gj_cols = (int) ceil((double) n / k);
    int Shj_cols = Gj_cols + 1;
    int Sh_size = n * Shj_cols; // size in number of bytes of matrix; n rows, 1 is for vector V
    int divisor = Sh_size * 8; // 8 bytes from preSh are need for each byte of Sh_i
    int divisor_bytes_segments = shadow_bmps_recovery[0]->header.info.image_size / divisor;

    // Current recovered secret bmp byte index
    int current_recovered_secret_byte_index = 0;

    // Current recovered watermark bmp byte index
    int current_recovered_wp_byte_index = 0;

    // Iterate for divisor bytes segments
    for (int segment_of_divisor_bytes = 0;
         segment_of_divisor_bytes < divisor_bytes_segments; segment_of_divisor_bytes++) {

        // Initialize G vector of k matrices Gj
        long ***matG = (long ***) malloc(k * sizeof(long **)); //TODO: free

        // Initialize B vector of k vectors vj
        long **matB = (long **) malloc(k * sizeof(long *)); //TODO: free

        // Iterate for each shadow bmp file
        for (int shadow_bmp_index = 0; shadow_bmp_index < k; shadow_bmp_index++) {

            int row = 0;
            int col = 0;

            // Iterate the bytes of the segment of this shadow bmp
            for (int b = 0; b < divisor; b++) {
                uint8_t curr_shadow_lsb_byte = (uint8_t) (shadow_bmps_recovery[shadow_bmp_index]->data[b +
                                                                                                       (segment_of_divisor_bytes *
                                                                                                        divisor)]);

                uint8_t one_or_zero = (uint8_t) (curr_shadow_lsb_byte & 0x01);

                matSh[shadow_bmp_index][row][col] = (uint8_t) ((matSh[shadow_bmp_index][row][col] << 1) | one_or_zero);

                if ((b + 1) % 8 == 0) {
                    // Reverse byte for correct writing to bmp
                    matSh[shadow_bmp_index][row][col] = reverse((uint8_t) matSh[shadow_bmp_index][row][col]);

                    if (matSh[shadow_bmp_index][row][col] > 250) {
                        int ke = 0;
                    }

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

            long **Sh = matSh[shadow_bmp_index];

            // Por cada Sh desconcatenamos V_t y G_t
            long **recoveredG = deconcatG(Sh, n, k);
            long *recoveredV = deconcatV(Sh, n);

            matG[shadow_bmp_index] = recoveredG;
            matB[shadow_bmp_index] = recoveredV;
        }

        // Matrix with 1s at first column and cjs at second column
        long **matCj = matrixCj(k, k);
//        printMatrix(k, k, matCj, "matCj:");

        // Initialize recovered R
        long **recoveredR = (long **) malloc(n * sizeof(long *));
        for (int i = 0; i < n; i++)
            recoveredR[i] = (long *) calloc((size_t) n, sizeof(long));

        // Initialize Gxy matrix (but it's really a vector)
        long **Gxy = (long **) malloc(k * sizeof(long *));
        for (int i = 0; i < k; i++)
            Gxy[i] = (long *) calloc(1, sizeof(long));

        // Traverse all positions of G matrices
        for (int x = 0; x < n; x++) {
            for (int y = 0; y < (int) ceil((double) n / k); y++) {
                // Get current G for x and y
                Gxy = resultG(x, y, matG, k);

//                printf("\n***");
//                printf("\nGxy con x:%d | y:%d", x, y);
//                printMatrix(1, k, Gxy, "");
//                printf("***\n");

                // Calculate solution vector
                //long **concatCjGs = concatMatMat(matCj, Gxy, k, k, (int) ceil((double) n / k)); //ORIGINAL
                long **concatCjGs = concatMatMat(matCj, Gxy, k, k, 1);
//                printMatrix(k + 1, k, concatCjGs, "la concatCjGs:");

                long *solutionVector = gaussJordan(k, concatCjGs, inverses);

//                printVector(k, solutionVector, "solutionVector:");

                // Save solution vector at recovered R
                recoveredR[x][0 + k * y] = solutionVector[0];
                recoveredR[x][1 + k * y] = solutionVector[1];
                recoveredR[x][2 + k * y] = solutionVector[2];
                recoveredR[x][3 + k * y] = solutionVector[3];

                free(solutionVector);
            }
        }

//        printMatrix(n, n, recoveredR, "Recovered R:");

        // Matrix Recovered V
        matB = transpose(matB, k, n);
//        printMatrix(k, n, matB, "Recovered V (B):");

        // Matrix Recovered Sd
        long **Sd = projectionSd(matB, n, k, inverses);
        printMatrix(n, n, Sd, "Recovered Sd");

        // Matrix Recovered S
        long **recoveredS = add(Sd, recoveredR, n);
//        printMatrix(n, n, recoveredS, "Pre Recovered S:");

        long **currentRw = convertUint8StreamToLongMatrix(rw_bmp->data + (current_recovered_wp_byte_index * n * n), n,
                                                          n);
        current_recovered_wp_byte_index++;

        // Matrix Recovered W
        long **recoveredW = add(Sd, currentRw, n);
//        printMatrix(n, n, recoveredW, "Recovered W:");

        // Fill recovered S to bmp
        for (int p = 0; p < n; p++) {
            for (int q = 0; q < n; q++) {
                // Set bit on rw_bmp->data
                recovered_secret_bmp->data[current_recovered_secret_byte_index] = (uint8_t) recoveredS[p][q];


                if (recovered_secret_bmp->data[current_recovered_secret_byte_index] > 250) {
                    int estamos_en_problemas = 1;
                }

                if (recovered_secret_bmp->data[current_recovered_secret_byte_index] !=
                    secret_bmp->data[current_recovered_secret_byte_index]) {
                    int estamos_en_problemas = 1;
                }

                // Save recovered W
                recovered_wm_bmp->data[current_recovered_secret_byte_index] = (uint8_t) recoveredW[p][q];

                // Advance current byte index, used for both
                current_recovered_secret_byte_index++;
            }
        }
    }
}

void recover(int n,
             int k,
             int *inverses,
             char *retrievedImage,
             char *watermarkTransformationImage,
             char *output_dir) {

    // Create output/lsb directory
    char output_lsb_recovery_dir[MAX_PATH];
    strcat(output_lsb_recovery_dir, "./");
    strcat(output_lsb_recovery_dir, output_dir);
    strcat(output_lsb_recovery_dir, "lsb");

    // Shadow bmp recovery array
    BITMAP_FILE *shadow_bmps_recovery[n];

    // Get shadow bmp files at output directory
    char **shadow_bmp_output_files = get_shadow_files(output_lsb_recovery_dir, n);

    // Recover shadow bmps
    for (int t = 0; t < k; t++) {
        // Load shadow bmp modified at distribution by LSB
        shadow_bmps_recovery[t] = load_BMP(shadow_bmp_output_files[t]);

        // If image size is not the same as secret's, abort
        if (shadow_bmps_recovery[t]->header.info.width != shadow_bmps_recovery[0]->header.info.width
            || shadow_bmps_recovery[t]->header.info.height != shadow_bmps_recovery[0]->header.info.height) {
            errx(EXIT_FAILURE, "A share bmp width or height does not equal secret's.");
        }
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


    // TODO: secret_bmp should not be passed to run, only for debugging
    // Load Rw bmp
    BITMAP_FILE *secret_bmp = load_BMP(watermarkTransformationImage);

    // Recover secret in shadows and watermark
    run(n, k, inverses, shadow_bmps_recovery, rw_bmp, recovered_secret_bmp, recovered_wm_bmp, secret_bmp);

    // Save Recovered_Secret.bmp
    write_BMP(recovered_secret_bmp);

    // Save Recovered_Watermark.bmp
    write_BMP(recovered_wm_bmp);

    // Destroy resources
    freeCharMatrix(shadow_bmp_output_files, n);
}

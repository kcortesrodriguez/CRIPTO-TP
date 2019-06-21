#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "modular.h"
#include "io.h"
#include "distribution.h"
#include "matrix.h"
#include "bitmap.h"
#include "random.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include "global.h"
#include "recovery.h"

int VERBOSE = FALSE;

int main(int argc, char *argv[]) {

    char secretImage[MAX_LEN];
    char retrievedImage[MAX_LEN];
    char watermarkImage[MAX_LEN];
    char watermarkTransformationImage[MAX_LEN];
    char shadowDirectory[MAX_LEN];
    int k;
    int n;

    // Create output directory
    char output_dir[MAX_PATH];
    strcat(output_dir, "output/");
    createDirectory(output_dir);

    // Create output/lsb directory
    char output_lsb_dir[MAX_PATH];
    strcat(output_lsb_dir, "./");
    strcat(output_lsb_dir, output_dir);
    strcat(output_lsb_dir, "lsb/");
    createDirectory(output_lsb_dir);

    // Parse parameters
    parseParameters(argc, argv,
                    sizeof secretImage / sizeof *secretImage,
                    secretImage, retrievedImage,
                    watermarkImage, watermarkTransformationImage,
                    shadowDirectory,
                    &k, &n);

    // Populate multiplicative inverses mod 251
    int *inverses = modularInverse(250, 251);

    // Shadow bmp array
    BITMAP_FILE *shadow_bmps[n];

    // Get shadow files at directory
    char **shadow_files = get_shadow_files(shadowDirectory, n);

    // Initialize shadow bmp files
    initialize_shadow_bmp_files(n, shadow_files, shadow_bmps, output_lsb_dir);

    // Initialize shadow bmp index array
    int shadow_bmps_index[n];
    memset(shadow_bmps_index, 0, n * sizeof(int));

    // Load secret bmp
    BITMAP_FILE *secret_bmp = load_BMP(secretImage);

    // Load watermark bmp
    BITMAP_FILE *w_bmp = load_BMP(watermarkImage);

    // Create Rw bmp
    char rw_bmp_name[MAX_PATH];
    memset(rw_bmp_name, 0, strlen(rw_bmp_name));
    strcat(rw_bmp_name, output_dir);
    strcat(rw_bmp_name, "RW.bmp");
    BITMAP_FILE *rw_bmp = create_BMP(rw_bmp_name, w_bmp->header.info.width, w_bmp->header.info.height, 8);

    // Distribute secret in shadows and Rw
    distribute(n, k, inverses, secret_bmp, w_bmp, rw_bmp, shadow_bmps_index, shadow_bmps);

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
//    freeCharMatrix(shadow_files, n); // Free for distribution
    for (int t = 0; t < n; t++) {
        destroy_BMP(shadow_bmps[t]);
    }


    /* END DISTRIBUTION */


    /* START RECOVERY */

    // Create output/lsb directory
    char output_lsb_recovery_dir[MAX_PATH];
    strcat(output_lsb_recovery_dir, "./");
    strcat(output_lsb_recovery_dir, output_dir);
    strcat(output_lsb_recovery_dir, "lsb");

    // Shadow bmp recovery array
    BITMAP_FILE *shadow_bmps_recovery[n];

    // Get shadow bmp files at output directory
    char **shadow_bmp_output_files = get_shadow_files(output_lsb_recovery_dir, n);

    // Initialize Sh vector of k matrices Shj
    long ***matSh = (long ***) malloc(k * sizeof(long **)); //TODO: free

    // Recover shadow bmps
    for (int t = 0; t < k; t++) {
        // Load shadow bmp modified at distribution by LSB
        shadow_bmps_recovery[t] = load_BMP(shadow_bmp_output_files[t]);

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
    int divisor_bytes_segments = shadow_bmps[0]->header.info.image_size / divisor;

    divisor_bytes_segments = 1; //TODO: remove

    // Iterate for divisor bytes segments
    for (int segment_of_divisor_bytes = 0;
         segment_of_divisor_bytes < divisor_bytes_segments; segment_of_divisor_bytes++) {

        // Initialize G vector of k matrices Gj
        long ***matG = (long ***) malloc(k * sizeof(long **)); //TODO: free

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
        }

        // Matrix with 1s at first column and cjs at second column
        //long **matCj = matrixCj(k); //DEPRECATED
        long **matCj = matrixCjV2(k,k); //NEW

        printMatrix(k,k,matCj, "matCj:");


        // Initialize recovered R
        long **recoveredR = (long **) malloc(n * sizeof(long *));
        for (int i = 0; i < n; i++)
            recoveredR[i] = (long *) calloc((size_t) n, sizeof(long));

        // Initialize Gxy matrix (but it's really a vector)
        long **Gxy = (long **) malloc(n * sizeof(long *));
        for (int i = 0; i < n; i++)
            Gxy[i] = (long *) calloc(1, sizeof(long));

        // Traverse all positions of G matrices
        for (int x = 0; x < n; x++) {
            //for (int y = 0; y < 4; y++) {
            for (int y = 0; y < (int) ceil((double) n / k); y++) {
                // Get current G for x and y
                Gxy = resultG(x, y, matG, k);

                printf("\n***");
                printf("\nGxy con x:%d | y:%d", x, y);
                printMatrix(1, k, Gxy, "");
                printf("***\n");

                // Calculate solution vector
                //long **concatCjGs = concatMatMat(matCj, Gxy, k, k, (int) ceil((double) n / k)); //ORIGINAL
                long **concatCjGs = concatMatMat(matCj, Gxy, k, k, 1);
                printMatrix(k + 1, k, concatCjGs, "la concatCjGs:"); //TODO: CHEQUEAR porque es -1

                long *solutionVector = gaussJordan(k, concatCjGs, inverses);

                printVector(2, solutionVector, "solutionVector:");

                // Save solution vector at recovered R
                recoveredR[x][y * 2] = solutionVector[0];
                recoveredR[x][y * 2 + 1] = solutionVector[1];
            }
        }

        printMatrix(n, n, recoveredR, "Recovered R:");
    }







//    for (int i = 0; i < sh_matrices; i++) {
//
//        // Initialize G vector of k matrices Gj
//        long ***matG = (long ***) malloc(k * sizeof(long **)); //TODO: free
//
//        // Traverse each participant image
//        for (int t = 0; t < k; t++) {
//
//            int row = shadow_bmps_recover_index[t] / Shj_cols;
//            int col = shadow_bmps_recover_index[t] % Shj_cols;
//
//            for (int b = 0; b < divisor; b++) {
//                uint8_t curr_shadow_lsb_byte = (uint8_t) (shadow_bmps[t]->data[b + (i * divisor)]);
//                uint8_t one_or_zero = (uint8_t) (curr_shadow_lsb_byte & 0x01);
//
//                matSh[t][row][col] = (uint8_t) ((matSh[t][row][col] << 1) | one_or_zero);
//
//                if ((b + 1) % 8 == 0) {
//                    col++; // Advance to next Shj col
//                    if ((col % Shj_cols) == 0) {
//                        col = 0;
//                        row++; // Advance to next Shj row
//                    }
//                    if ((row % n) == 0) // Advance to next Shj
//                        break;
//                }
//            }
//
//            shadow_bmps_recover_index[t]++;
//
//            long **Sh = matSh[t];
//
//            if (t == 0)
//                printMatrix(Shj_cols, n, Sh, "Sh matrix");
//
//            // Por cada Sh desconcatenamos V_t y G_t
////            long **recoveredG = deconcatG(Sh, n, k);
////            long *recoveredV = deconcatV(Sh, n);
////
////            matG[t] = recoveredG;
//        }


    /*

        // Matrix with 1s at first column and cjs at second column
        long **matCj = matrixCj(k);

        // Initialize recovered R
        long **recoveredR = (long **) malloc(n * sizeof(long *));
        for (i = 0; i < n; i++)
            recoveredR[i] = (long *) calloc((size_t) n, sizeof(long));

        // Initialize Gxy matrix (but it's really a vector)
        long **Gxy = (long **) malloc(n * sizeof(long *));
        for (i = 0; i < n; i++)
            Gxy[i] = (long *) calloc(1, sizeof(long));

        // Traverse all positions of G matrices
        for (int x = 0; x < n; x++) {
            for (int y = 0; y < (int) ceil((double) n / k); y++) {
                // Get current G for x and y
                Gxy = resultG(x, y, matG, k);

                // Calculate solution vector
                long **concatCjGs = concatMatMat(matCj, Gxy, k, 2, (int) ceil((double) n / k));
                long *solutionVector = gaussJordan(k, concatCjGs, inverses);

//                printVector(2, solutionVector, "solutionVector:");

                // Save solution vector at recovered R
                recoveredR[x][y * 2] = solutionVector[0];
                recoveredR[x][y * 2 + 1] = solutionVector[1];
            }
        }

//        printMatrix(n, n, recoveredR, "Recovered R:");
    }

     */


    //Start desencryption example from paper

//    k = 2;
//    n = 4;
//
//    long **B = (long **) malloc(n * sizeof(long *));
//    for (int i = 0; i < n; i++)
//        B[i] = (long *) calloc((size_t) k, sizeof(long));
//
//    B[0][0] = 62L;
//    B[0][1] = 40L;
//    B[1][0] = 59L;
//    B[1][1] = 28L;
//    B[2][0] = 43L;
//    B[2][1] = 28L;
//    B[3][0] = 84L;
//    B[3][1] = 48L;
//
//    printMatrix(k, n, B, "B Matrix");
//
//    long **newSd = projectionSd(B, n, k, inverses);
//
//    printMatrix(n, n, newSd, "Reconstruction Sd Matrix from B Matrix:");
//
    // Destroy resources
//    freeLongMatrix(newSd, n);
//    freeLongMatrix(B, n);
    free(inverses);
    freeCharMatrix(shadow_files, n); // Free from recovery

    return EXIT_SUCCESS;
}


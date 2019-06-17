#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "modular.h"
#include "io.h"
#include "distribution.h"
#include "matrix.h"
#include "bmp.h"
#include "bitmap.h"
#include "random.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#define MAX_LEN 260

int main(int argc, char *argv[]) {

    char secretImage[MAX_LEN];
    char retrievedImage[MAX_LEN];
    char watermarkImage[MAX_LEN];
    char watermarkTransformationImage[MAX_LEN];
    char directory[MAX_LEN];
    int k;
    int n;

    // Create output directory
    char output_dir[MAX_PATH];
    strcat(output_dir, "output/");
    createDirectory(output_dir);

    // Create output/lsb directory
    char output_lsb_dir[MAX_PATH];
    strcat(output_lsb_dir, output_dir);
    strcat(output_lsb_dir, "lsb/");
    createDirectory(output_lsb_dir);

    // Parse parameters
    parseParameters(argc, argv,
                    sizeof secretImage / sizeof *secretImage,
                    secretImage, retrievedImage,
                    watermarkImage, watermarkTransformationImage,
                    directory,
                    &k, &n);

    // Get shadow files at directory
    char **shadow_files = get_shadow_files(directory, n);

    // Shadow bmp array
    BITMAP_FILE *shadow_bmps[n];

    // Shadow bmp index array
    int shadow_bmps_index[n];
    memset(shadow_bmps_index, 0, n * sizeof(int));

    for (int t = 0; t < n; t++) {
        char *shadow_bmp_path = shadow_files[t];

        BITMAP_FILE *shadow_bmp = load_BMP(shadow_bmp_path);

        char lsb_shadow_bmp_name[MAX_PATH];
        memset(lsb_shadow_bmp_name, 0, strlen(lsb_shadow_bmp_name));
        strcat(lsb_shadow_bmp_name, output_lsb_dir);
        strcat(lsb_shadow_bmp_name, basename(shadow_bmp_path));

        memset(shadow_bmp->fname, 0, sizeof shadow_bmp->fname);
        strcpy(shadow_bmp->fname, lsb_shadow_bmp_name);

        shadow_bmp->header.file.res1 = (unsigned short) (t + 1);

        shadow_bmps[t] = shadow_bmp;
    }

    // Populate multiplicative inverses mod 251
    int *inverses = modularInverse(250, 251);

    // Load secret bmp
    BITMAP_FILE *secret_bmp = load_BMP(secretImage);

    // Matrix W
    // Load watermark bmp
    BITMAP_FILE *watermark_bmp = load_BMP(watermarkImage);

    // Create Rw bmp
    char rw_bmp_name[MAX_PATH];
    memset(rw_bmp_name, 0, strlen(rw_bmp_name));
    strcat(rw_bmp_name, output_dir);
    strcat(rw_bmp_name, "RW.bmp");
    BITMAP_FILE *rw_bmp = create_BMP(rw_bmp_name, watermark_bmp->header.info.width, watermark_bmp->header.info.height,
                                     8);
    check_BMP(rw_bmp);
    int current_rw_byte_index = 0;

    // Traverse 8 bpp secret image n x n bytes at a time
    int s_matrices = secret_bmp->header.info.image_size / (n * n);
    for (int i = 0; i < s_matrices; i++) {

        // Matrix S
        // Convert secret stream to n x n matrix
        long **S = convertUint8StreamToLongMatrix(secret_bmp->data + (i * n * n), n, n);

        printf("\n");
        printf("S matrix:\n");
        printMatrix(n, n, S);

        // Matrix A
        long **A = matA(n, k);

//        printf("\nA matrix:\n");
//        printMatrix(k, n, A);

        // Matrix Sd
        long **Sd = projectionSd(A, n, k, inverses);

//        printf("\nSd matrix:\n");
//        printMatrix(n, n, Sd);

        // Matrix R
        long **R = remainderR(S, Sd, n);

//        printf("\nR matrix:\n");
//        printMatrix(n, n, R);

        // Recovered Matrix S
//        long **recoveredS = add(R, Sd, n);
//        printf("\n");
//        printf("Recovered S matrix:\n");
//        printMatrix(n, n, recoveredS);

        // Convert watermark stream to n x n matrix
        long **W = convertUint8StreamToLongMatrix(watermark_bmp->data + (i * n * n), n, n);

        // Matrix Rw
        long **Rw = remainderRw(W, Sd, n);
        printf("\nRw matrix:\n");
        printMatrix(n, n, Rw);

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
//    printf("\nX matrix:\n");
//    printMatrix(n, k, matrix);
//
        // Matrix V
        long **V = matV(A, X, n, k);
//    printf("\nV matrix:\n");
//    printMatrix(n, n, V);

        // Matrix G
        long ***G = matG(R, n, k);
//        printf("\nG matrix:\n");
//        for (int t = 0; t < n; t++) {
//            printf("G_%d matrix:\n", t + 1);
//            printMatrix((int) ceil((double) n / k), n, G[t]);
//        }

        // Matrix Sh
//        printf("\nSh matrix:\n");
        long ***Sh = matSh(G, V, n, k);
        uint8_t ***uint8_Sh = (uint8_t ***) malloc(n * sizeof(uint8_t **));
        for (int t = 0; t < n; t++) {
//            printf("Sh_%d matrix:\n", t + 1);
//            printMatrix((int) (ceil((double) n / k) + 1), n, Sh[t]);
            uint8_Sh[t] = convertMatrixFromLongToUint8(Sh[t], n, (int) (ceil((double) n / k) + 1));
        }

        for (int t = 0; t < n; t++) {

            // Recorremos la peli un byte a la vez
            int current_movie_byte_index = shadow_bmps_index[t];

            if (t == 0)
                printf("current_movie_byte_index:\t%d\n", current_movie_byte_index);

            // Recorremos Sh 1 un byte a la vez, agarrando para cada uno cada bit (0 a 7)
            for (int a = 0; a < n; a++) {
                for (int b = 0; b < (int) (ceil((double) n / k) + 1); b++) {
                    uint8_t current_byte = uint8_Sh[t][a][b];

                    // Bit por bit, cada byte de Sh1
                    for (int l = 0; l < 8; l++) {
                        uint8_t one_or_zero = (uint8_t) ((current_byte >> l) & 0x01);

                        // Set bit on movie
                        uint8_t current_movie_byte = (uint8_t) shadow_bmps[t]->data[current_movie_byte_index];
                        shadow_bmps[t]->data[current_movie_byte_index] =
                                (uint8_t) (current_movie_byte & ~1) | one_or_zero;
                        current_movie_byte_index++;
                    }
                }
            }

            shadow_bmps_index[t] = current_movie_byte_index;

//            freeMatrix(uint8_Sh[t], n);
        }

        freeMatrix(A, n);
        freeMatrix(Sd, n);
        freeMatrix(S, n);
        freeMatrix(R, n);
        for (int t = 0; t < n; t++) {
            freeMatrix(G[t], n);
        }
        free(G);
//        freeMatrix(recoveredS, n);
        //            freeMatrix(Sh[t], n);
        //            freeMatrix(Sh, n);
    }

    // Save shadow bmps
    for (int t = 0; t < n; t++) {
        write_BMP(shadow_bmps[t]);
    }

    // Save Rw.bmp
    write_BMP(rw_bmp);

    // Destroy resources
    free(inverses);

    return 0;
}

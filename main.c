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
    strcat(output_dir, "./output");
    createDirectory(output_dir);

    // Parse parameters
    parseParameters(argc, argv,
                    sizeof secretImage / sizeof *secretImage,
                    secretImage, retrievedImage,
                    watermarkImage, watermarkTransformationImage,
                    directory,
                    &k, &n);

    // Populate multiplicative inverses mod 251
    int *inverses = modularInverse(250, 251);

    // Matrix S
    // Load secret bmp
    BITMAP_FILE *secret_bmp = load_BMP(secretImage);

    // Matrix W
    // Load watermark bmp
    BITMAP_FILE *watermark_bmp = load_BMP(watermarkImage);

    // Create Rw bmp
    char rw_bmp_name[MAX_PATH];
    memset(rw_bmp_name, 0, strlen(rw_bmp_name));
    strcat(rw_bmp_name, output_dir);
    strcat(rw_bmp_name, "/RW.bmp");
    BITMAP_FILE *rw_bmp = create_BMP(rw_bmp_name, watermark_bmp->header.info.width, watermark_bmp->header.info.height,
                                     8);
    check_BMP(rw_bmp);
    int current_rw_byte_index = 0;

    // Traverse 8 bpp secret image n x n bytes at a time
    int s_matrices = secret_bmp->header.info.image_size / (n * n);
    for (int i = 0; i < s_matrices; i++) {

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

        for (int p = 0; p < n; p++) {
            for (int q = 0; q < n; q++) {
                // Set bit on rw_bmp->data
                rw_bmp->data[current_rw_byte_index] = (uint8_t) Rw[p][q];
                current_rw_byte_index++;
            }
        }
    }

    // Save Rw.bmp
    write_BMP(rw_bmp);

    // Matrix X
//    printf("\n");
//    printf("\nX matrix:\n");
//    long **matrix = matX(k, n);
//    printMatrix(n, k, matrix);
//
//    // Matrix V
//    long **V = matV(A, matrix, n, k);
//
//    printf("\n");
//    printf("V matrix:\n");
//    printMatrix(n, n, V);
//
//    // Matrix G
//    printf("\n");
//    printf("G matrix:\n");
//    long ***G = matG(R, n, k);
//    for (int t = 0; t < n; t++) {
//        printf("G_%d matrix:\n", t + 1);
//        printMatrix((int) ceil((double) n / k), n, G[t]);
//    }
//
//    // Matrix Sh
//    printf("\n");
//    printf("Sh matrix:\n");
//    long ***Sh = matSh(G, V, n, k);
//    uint8_t ***uint8_Sh = (uint8_t ***) malloc(n * sizeof(uint8_t **));
//    for (int t = 0; t < n; t++) {
//        printf("Sh_%d matrix:\n", t + 1);
//        printMatrix((int) (ceil((double) n / k) + 1), n, Sh[t]);
//        uint8_Sh[t] = convertMatrixFromLongToUint8(Sh[t], n, (int) (ceil((double) n / k) + 1));
//    }
//
//    // Get shadow files at directory
//    char **shadow_files = get_shadow_files(directory, n);
//
//    for (int t = 0; t < n; t++) {
//        char *shadow_bmp_path = shadow_files[t];
//
//        BITMAP_FILE *shadow_bmp = load_BMP(shadow_bmp_path);
//
//        char lsb_shadow_bmp_name[MAX_PATH];
//        memset(lsb_shadow_bmp_name, 0, strlen(lsb_shadow_bmp_name));
//        strcat(lsb_shadow_bmp_name, directory);
//        strcat(lsb_shadow_bmp_name, "/lsb/");
//        strcat(lsb_shadow_bmp_name, basename(shadow_bmp_path));
//
//        memset(shadow_bmp->fname, 0, sizeof shadow_bmp->fname);
//        strcpy(shadow_bmp->fname, lsb_shadow_bmp_name);
//
//        shadow_bmp->header.file.res1 = (unsigned short) (t + 1);
//
//        // Recorremos la peli un byte a la vez
//        int current_movie_byte_index = 0;
//
//        // Recorremos Sh 1 un byte a la vez, agarrando para cada uno cada bit (0 a 7)
//        for (int i = 0; i < n; i++) {
//            for (int j = 0; j < (k + 1); j++) {
//                uint8_t current_byte = uint8_Sh[t][i][j];
//
//                // Bit por bit, cada byte de Sh1
//                for (int l = 0; l < 8; l++) {
//                    uint8_t one_or_zero = (uint8_t) ((current_byte >> l) & 0x01);
//
//                    // Set bit on movie
//                    uint8_t *current_movie_byte = shadow_bmp->data + current_movie_byte_index;
//                    (*current_movie_byte) = (uint8_t) (*current_movie_byte & ~1) | one_or_zero;
//                    current_movie_byte_index++;
//                }
//            }
//        }
//
//        write_BMP(shadow_bmp, 0);
//    }
//
//    free(inverses);
//    freeMatrix(A, n);
//    freeMatrix(Sd, n);
//    freeMatrix(recoveredS, n);
//    freeMatrix(S, n);
//    freeMatrix(R, n);
//    for (int t = 0; t < n; t++) {
//        freeMatrix(G[t], n);
//    }
//    free(G);
//    free(Sh);

    return 0;
}

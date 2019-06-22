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
    bool isDistribute;

    // Parse parameters
    parseParameters(argc, argv,
                    sizeof secretImage / sizeof *secretImage,
                    secretImage, retrievedImage,
                    watermarkImage, watermarkTransformationImage,
                    shadowDirectory,
                    &k,
                    &n,
                    &isDistribute);

    // Create output directory
    char output_dir[MAX_PATH];
    strcat(output_dir, "output/");
    createDirectory(output_dir);

    // Populate multiplicative inverses mod 251
    int *inverses = modularInverse(250, 251);

    if (isDistribute) {

        // Create output/lsb directory
        char output_lsb_dir[MAX_PATH];
        strcat(output_lsb_dir, "./");
        strcat(output_lsb_dir, output_dir);
        strcat(output_lsb_dir, "lsb/");
        createDirectory(output_lsb_dir);

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
        freeCharMatrix(shadow_files, n); // Free for distribution
        for (int t = 0; t < n; t++) {
            destroy_BMP(shadow_bmps[t]);
        }

    } else {

    }

    // Destroy resources
    free(inverses);

    return EXIT_SUCCESS;
}


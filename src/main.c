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

        distribute(
                n,
                k,
                inverses,
                secretImage,
                watermarkImage,
                output_dir,
                shadowDirectory); // Where shares reside

//    } else { //TODO: remove comment

        recover(
                n,
                k,
                inverses,
                "./output/Secreto.bmp",
                "./output/Rw.bmp",
                output_dir); // Where shares modified with LSB will be saved
    }

    // Destroy resources
    free(inverses);

    return EXIT_SUCCESS;
}


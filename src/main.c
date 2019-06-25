#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "modular.h"
#include "io.h"
#include "distribution.h"
#include "random.h"
#include <sys/time.h>
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
    DIR *output_dir_struct = createDirectory(output_dir);

    // Populate multiplicative inverses mod 251
    int *inverses = modularInverse(250, 251);

    // Set seed for random chars
    set_seed(time(NULL));

    if (isDistribute) {

        distribute(
                n,
                k,
                inverses,
                secretImage,
                watermarkImage,
                output_dir,
                shadowDirectory); // Where shares modified with LSB will be saved

    } else {

        recover(n,
                k,
                inverses,
                retrievedImage,
                watermarkTransformationImage,
                output_dir,
                shadowDirectory); // Where shares reside
    }

    // Destroy resources
    free(inverses);
    closedir(output_dir_struct);

    return EXIT_SUCCESS;
}


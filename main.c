#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "modular.h"
#include "io.h"
#include "distribution.h"
#include "matrix.h"

#define MAX_LEN 100

int main(int argc, char *argv[]) {

    int inverses[251];

    char secretImage[MAX_LEN];
    char retrievedImage[MAX_LEN];
    char watermarkImage[MAX_LEN];
    char watermarkTransformationImage[MAX_LEN];
    char directory[MAX_LEN];
    int k;
    int n;

    // Parse parameters
    // e.g.: -d -s Albert.bmp -m Paris.bmp -k 4 -n 8 --dir color280x440/
    parseParameters(argc, argv,
                    sizeof secretImage / sizeof *secretImage,
                    secretImage, retrievedImage,
                    watermarkImage, watermarkTransformationImage,
                    directory,
                    &k, &n);
//
//    // Populate multiplicative inverses mod 251
//    modularInverse(250, 251, inverses);



    matA(n, k);


    //start testing Matrix X (Kevin)
    k = 4;
    n = 8;

    long** matrix;
    printf("\n");
    matrix = generateMatrixX(k,n);
    printMatrix(k,n,matrix);

    return 0;
}
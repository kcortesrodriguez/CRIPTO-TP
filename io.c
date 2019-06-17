#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <memory.h>
#include <stdlib.h>
#include <err.h>
#include "global.h"

void parseParameters(int argc, char *argv[],
                     size_t size,
                     char *secretImage, char *retrievedImage,
                     char *watermarkImage, char *watermarkTransformationImage,
                     char *directory,
                     int *k, int *n) {

    struct option longopts[] = {
            {"dir", required_argument, NULL, 'i'},
            {0, 0, 0,                        0}
    };

    const char *usageFormat = "Usage: %s [-d | -r] -s filename.bmp -m filename.bmp -k -n --dir directory\n";

    bool distribute = false;
    bool retrieve = false;
    int opt;

    while ((opt = getopt_long(argc, argv, "drs:m:k:n:i:v", longopts, NULL)) != -1) {
        switch (opt) {
            case 'v':
                VERBOSE = TRUE;
                break;
            case 'd':
                distribute = true;
                break;
            case 'r':
                retrieve = true;
                break;
            case 's':
                if (distribute) {
                    strncpy(secretImage, optarg, size - 1);
                    secretImage[size - 1] = 0;

                    // Verify secret image filename format is bmp
                    char *dot = strrchr(secretImage, '.');
                    if (!dot || strcmp(dot, ".bmp") != 0) {
                        fprintf(stderr, "Filename is not bmp a file.\n");
                        fprintf(stderr, usageFormat, argv[0]);
                        exit(EXIT_FAILURE);
                    }

                } else if (retrieve) {
                    strncpy(retrievedImage, optarg, size - 1);
                    retrievedImage[size - 1] = 0;

                    // Verify secret image filename format is bmp
                    char *dot = strrchr(retrievedImage, '.');
                    if (!dot || strcmp(dot, ".bmp") != 0) {
                        fprintf(stderr, "Filename is not bmp a file.\n");
                        fprintf(stderr, usageFormat, argv[0]);
                        exit(EXIT_FAILURE);
                    }

                }
                break;
            case 'm':
                if (distribute) {
                    strncpy(watermarkImage, optarg, size - 1);
                    watermarkImage[size - 1] = 0;

                    // Verify water mark filename format is bmp
                    char *dot = strrchr(watermarkImage, '.');
                    if (!dot || strcmp(dot, ".bmp") != 0) {
                        fprintf(stderr, "Filename is not bmp a file.\n");
                        fprintf(stderr, usageFormat, argv[0]);
                        exit(EXIT_FAILURE);
                    }

                } else if (retrieve) {
                    strncpy(watermarkTransformationImage, optarg, size - 1);
                    watermarkTransformationImage[size - 1] = 0;

                    // Verify water mark filename format is bmp
                    char *dot = strrchr(watermarkTransformationImage, '.');
                    if (!dot || strcmp(dot, ".bmp") != 0) {
                        fprintf(stderr, "Filename is not bmp a file.\n");
                        fprintf(stderr, usageFormat, argv[0]);
                        exit(EXIT_FAILURE);
                    }

                }
                break;
            case 'k':
                *k = atoi(optarg);
                break;
            case 'n':
                *n = atoi(optarg);
                break;
            case 'i':
                strncpy(directory, optarg, size - 1);
                directory[size - 1] = 0;
                break;
            default:
                fprintf(stderr, usageFormat, argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Check exclusive option: either distribute or retrieve
    if (distribute && retrieve) {
        errx(EXIT_FAILURE, "-d and -r cannot be used together");
    }

}



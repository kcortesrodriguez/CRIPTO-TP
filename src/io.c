#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <memory.h>
#include <stdlib.h>
#include <err.h>
#include <fts.h>
#include <sys/stat.h>
#include <libgen.h>
#include <dirent.h>
#include "global.h"

#define MAX_STRING 260

void parseParameters(int argc, char *argv[],
                     size_t size,
                     char *secretImage,
                     char *retrievedImage,
                     char *watermarkImage,
                     char *watermarkTransformationImage,
                     char *directory,
                     int *k,
                     int *n,
                     bool *isDistribute) {

    struct option longopts[] = {
            {"dir", required_argument, NULL, 'i'},
            {0, 0, 0,                        0}
    };

    const char *usageFormat = "Usage: %s [-d | -r] -s filename.bmp -m filename.bmp -k -n --dir directory\n";

    bool distribute = false;
    bool recover = false;
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
                recover = true;
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

                } else if (recover) {
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

                } else if (recover) {
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

    // Check 2 =< k <= n
    if (*k < 2 || *k > *n) {
        errx(EXIT_FAILURE, "2 =< k <= n");
    }

    // Check exclusive option: either distribute or recover
    if (distribute && recover) {
        errx(EXIT_FAILURE, "-d and -r cannot be used together");
    }

    // If distribute is true, isDistribute = true, otherwise the program will recover
    *isDistribute = distribute;
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

char **get_shadow_files(char *directory, int n) {

    // Initialize shadow_files
    char **shadow_files = malloc(sizeof(char *) * n);
    if (!shadow_files)
        return NULL;
    for (int i = 0; i < n; i++) {
        shadow_files[i] = malloc(MAX_STRING + 1);
        if (!shadow_files[i]) {
            free(shadow_files);
            return NULL;
        }
    }

    // Prepare array for fts_open
    char *path_to_shadows[2] = {directory, NULL};

    // Initialize fts_open arguments
    FTS *ftsp;
    FTSENT *p, *chp;
    int fts_options = FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR;
    if ((ftsp = fts_open(path_to_shadows, fts_options, NULL)) == NULL) {
        warn("fts_open");
        abort();
    }
    chp = fts_children(ftsp, 0);
    if (chp == NULL) {
        return NULL;    /* no files to traverse */
    }

    // Traverse directory
    int shadow_index = n - 1;
    while ((p = fts_read(ftsp)) != NULL && (shadow_index + 1)) {
        switch (p->fts_info) {
            case FTS_F:
                // No file at subfolders, only at present level
                if (p->fts_level == 1 && (strcmp("bmp", get_filename_ext(basename(p->fts_path))) == 0)) {
                    strncpy(shadow_files[shadow_index], p->fts_path, MAX_STRING);
                    shadow_index = shadow_index - 1;
                }
                break;
            default:
                break;
        }
    }

    fts_close(ftsp);

    return shadow_files;
}

void createDirectory(char *path) {

    int res1 = (int) opendir(path);
    if (res1 == 0) {
        int res2 = mkdir(path, 0777);
        if (res2 != 0) {
            char aux[MAX_LEN];
            sprintf(aux, "Could not create %s", path);
            errx(EXIT_FAILURE, aux);
        }
    }
}

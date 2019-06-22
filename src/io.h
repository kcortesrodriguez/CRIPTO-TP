#ifndef CRIPTO_TP_IO_H
#define CRIPTO_TP_IO_H

#include <stdbool.h>

void parseParameters(int argc, char *argv[],
                     size_t size,
                     char *secretImage, char *retrievedImage,
                     char *watermarkImage, char *watermarkTransformationImage,
                     char *directory,
                     int *k,
                     int *n,
                     bool *isDistribute);

char **get_shadow_files(char *dir, int n);

void createDirectory(char *path);

const char *get_filename_ext(const char *filename);

#endif //CRIPTO_TP_IO_H

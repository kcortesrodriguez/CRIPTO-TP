#ifndef CRIPTO_TP_IO_H
#define CRIPTO_TP_IO_H

void parseParameters(int argc, char *argv[],
                     size_t size,
                     char *secretImage, char *retrievedImage,
                     char *watermarkImage, char *watermarkTransformationImage,
                     char *directory,
                     int *k, int *n);

char **get_shadow_files(char *dir, int n);

void createDirectory(char *path);

#endif //CRIPTO_TP_IO_H

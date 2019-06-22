#ifndef CRIPTO_TP_RECOVERY_H
#define CRIPTO_TP_RECOVERY_H

#include "bitmap.h"

uint8_t reverse(uint8_t b);

long *gaussJordan(int n, long **matrix, int inverses[251]);

long **matrixCj(int j, int k);

long **resultG(int x, int y, long ***allGs, int totalGs);

void recover(int n,
             int k,
             int *inverses,
             char *retrievedImage,
             char *watermarkTransformationImage,
             char *output_dir);

#endif //CRIPTO_TP_RECOVERY_H

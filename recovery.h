#ifndef CRIPTO_TP_RECOVERY_H
#define CRIPTO_TP_RECOVERY_H

#include "bitmap.h"

uint8_t reverse(uint8_t b);

long *gaussJordan(int n, long **matrix, int inverses[251]);

long **matrixCj(int j, int k);

long **resultG(int x, int y, long ***allGs, int totalGs);

void recovery(int n,
              int k,
              int *inverses,
              BITMAP_FILE *secret_bmp,
              BITMAP_FILE *watermark_bmp,
              BITMAP_FILE *rw_bmp,
              BITMAP_FILE **shadow_bmps,
              char *output_dir);

#endif //CRIPTO_TP_RECOVERY_H

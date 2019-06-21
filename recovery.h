#ifndef CRIPTO_TP_RECOVERY_H
#define CRIPTO_TP_RECOVERY_H

#include "bitmap.h"

uint8_t reverse(uint8_t b);

long *gaussJordan(int n, long **matrix, int inverses[251]);

long **matrixCj(int j);

long **matrixCjV2(int j, int k);

long **resultG(int x, int y, long ***allGs, int totalGs);

void recover(int n,
             int k,
             int *inverses,
             BITMAP_FILE *secret_bmp,
             BITMAP_FILE *watermark_bmp,
             BITMAP_FILE *rw_bmp,
             int *shadow_bmps_index,
             BITMAP_FILE **shadow_bmps);

#endif //CRIPTO_TP_RECOVERY_H

#ifndef CRIPTO_TP_RECOVERY_H
#define CRIPTO_TP_RECOVERY_H

#include "bitmap.h"

uint8_t reverse(uint8_t b);

long *gaussJordan(int n, long **matrix, int inverses[251]);

long **matrixCj(int j, int k, BITMAP_FILE **shadow_bmps_recovery);

long **resultG(int x, int y, long ***allGs, int totalGs);

void recover(int n, int k, int *inverses, char retrievedImage[260], char watermarkTransformationImage[260],
             char output_dir[260], char string[260]);

#endif //CRIPTO_TP_RECOVERY_H

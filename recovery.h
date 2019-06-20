#ifndef CRIPTO_TP_RECOVERY_H
#define CRIPTO_TP_RECOVERY_H

#include "bitmap.h"

void recover(int n,
             int k,
             int *inverses,
             BITMAP_FILE *secret_bmp,
             BITMAP_FILE *watermark_bmp,
             BITMAP_FILE *rw_bmp,
             int *shadow_bmps_index,
             BITMAP_FILE **shadow_bmps);

#endif //CRIPTO_TP_RECOVERY_H

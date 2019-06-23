#ifndef CRIPTO_TP_DISTRIBUTION_H
#define CRIPTO_TP_DISTRIBUTION_H

#include "bitmap.h"

long **matA(int n, int k);

long **projectionSd(long **A, int n, int k, int inverses[251]);

long **remainderR(long **secretS, long **projectionSd, int n);

long **remainderRw(long **watermarkW, long **projectionSd, int n);

long *g_i_j(long **R, int initial_column, int t, int n, int k);

long **matG_t(long **R, int n, int k, int t);

long ***matG(long **R, int n, int k);

long **matV(long **A, long **X, int n, int k);

long ***matSh(long ***G, long **V, int n, int k);

long *generateVector(int k, int initialValue);

long **matX(int k, int n);

void initialize_shadow_bmp_files(int n,
                                 char **shadow_files,
                                 BITMAP_FILE **shadow_bmps,
                                 char *output_lsb_dir,
                                 unsigned int secret_width,
                                 unsigned int secret_height);

void truncateBytesTo250(long **S, int n);

void run_d(int n,
           int k,
           int *inverses,
           BITMAP_FILE *secret_bmp,
           BITMAP_FILE *watermark_bmp,
           BITMAP_FILE *rw_bmp,
           int *shadow_bmps_index,
           BITMAP_FILE **shadow_bmps);

void distribute(int n,
                int k,
                int *inverses,
                char *secretImage,
                char *watermarkImage,
                char *output_dir,
                char *shadowDirectory);

#endif //CRIPTO_TP_DISTRIBUTION_H

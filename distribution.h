#ifndef CRIPTO_TP_DISTRIBUTION_H
#define CRIPTO_TP_DISTRIBUTION_H

long **matA(int n, int k);

long **projectionSd(long **A, int n, int k, int inverses[251]);

long **remainderR(long **secretS, long **projectionSd, int n);

long **remainderRw(long **watermarkW, long **projectionSd, int n);

int *g_i_j(int **R, int initial_column, int t, int n, int k);

int **matG_t(int **R, int n, int k, int t);

int ***matG(int **R, int n, int k);

#endif //CRIPTO_TP_DISTRIBUTION_H

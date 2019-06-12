#ifndef CRIPTO_TP_DISTRIBUTION_H
#define CRIPTO_TP_DISTRIBUTION_H

int **matA(int n, int k);

int **projectionSd(int **A, int n, int k, int inverses[251]);

int **remainderR(int **secretS, int **projectionSd, int n);

int **remainderRw(int **watermarkW, int **projectionSd, int n);

int *g_i_j(int **R, int initial_column, int t, int n, int k);

int **matG_t(int **R, int n, int k, int t);

int ***matG(int **R, int n, int k);

#endif //CRIPTO_TP_DISTRIBUTION_H

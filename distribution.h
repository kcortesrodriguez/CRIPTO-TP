#ifndef CRIPTO_TP_DISTRIBUTION_H
#define CRIPTO_TP_DISTRIBUTION_H

long **matA(int n, int k);

long **projectionSd(long **A, int n, int k, int inverses[251]);

long **remainderR(long **secretS, long **projectionSd, int n);

long **remainderRw(long **watermarkW, long **projectionSd, int n);

long *g_i_j(long **R, int initial_column, int t, int n, int k);

long **matG_t(long **R, int n, int k, int t);

long ***matG(long **R, int n, int k);

long **matV(long **A, long **X, int n, int k);

long ***matSh(long ***G, long **V, int n, int k);

#endif //CRIPTO_TP_DISTRIBUTION_H

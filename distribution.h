#ifndef CRIPTO_TP_DISTRIBUTION_H
#define CRIPTO_TP_DISTRIBUTION_H

long **matA(int n, int k);

long **projectionSd(long **A, int n, int k, int inverses[251]);

long **remainderR(long **secretS, long **projectionSd, int n);

long **remainderRw(long **watermarkW, long **projectionSd, int n);

#endif //CRIPTO_TP_DISTRIBUTION_H

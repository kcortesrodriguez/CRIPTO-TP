#ifndef CRIPTO_TP_DISTRIBUTION_H
#define CRIPTO_TP_DISTRIBUTION_H

int **matA(int n, int k);

int **projectionSd(int **A, int n, int k, int inverses[251]);

int **remainderR(int **secretS, int **projectionSd, int n);

int **remainderRw(int **watermarkW, int **projectionSd, int n);

#endif //CRIPTO_TP_DISTRIBUTION_H

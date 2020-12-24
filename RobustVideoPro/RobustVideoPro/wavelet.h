#ifndef WAVELET_H
#define WAVELET_H
#include <stdio.h>
#include <stdlib.h>

void branch_lp_dn(double *signal, int length, double *sigop);
void branch_hp_dn(double *signal, int length, double *sigop);
void branch_lp_hp_up(double *sigL, int lengthL, double *sigH, int lengthH, double *oup);
void DWT2D(double **signal, int m, int n, double **cLL, double **cLH, double **cHL, double **cHH);
void IDWT2D(double **signal, int LLm, int LLn, double **cLL, double **cLH, double **cHL, double **cHH);
#endif // !WAVELET_H




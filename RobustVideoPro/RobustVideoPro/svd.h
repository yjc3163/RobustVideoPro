#pragma once
#ifndef SVD_H
#define SVD_H
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <stdbool.h>

double get_norm(double *x, int n);
double normalize(double *x, int n);
inline double product(double*a, double *b, int n);
void orth(double *a, double *b, int n);
bool svd(double **A, int rowsA, int colsA, int K, double **U, double *S, double **V);
#endif // !SVD_H

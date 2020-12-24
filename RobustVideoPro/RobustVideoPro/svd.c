#include "svd.h"

const int MAX_ITER = 100000;
const double eps = 0.0000001;

double get_norm(double *x, int n)
{
	double r = 0;
	int i;
	for (i = 0; i < n; i++)
		r += x[i] * x[i];
	return sqrt(r);
}

double normalize(double *x, int n)
{
	double r = get_norm(x, n);
	if (r < eps)
		return 0;
	int i;
	for (i = 0; i < n; i++)
		x[i] /= r;
	return r;
}

inline double product(double*a, double *b, int n)
{
	double r = 0;
	int i;
	for (i = 0; i < n; i++)
		r += a[i] * b[i];
	return r;
}

void orth(double *a, double *b, int n)
{
	//|a|=1    
	double r = product(a, b, n);
	int i;
	for (i = 0; i < n; i++)
		b[i] -= r*a[i];
}

bool svd(double **A, int rowsA, int colsA, int K, double **U, double *S, double **V)
{
	int M = rowsA;
	int N = colsA;

	int i, j;
	for (i = 0; i < K; i++) {
		S[i] = 0;
	}
	for (i = 0; i < K; i++) {
		for (j = 0; j < M; j++) {
			U[i][j] = 0;
		}
	}
	for (i = 0; i < K; i++) {
		for (j = 0; j < N; j++) {
			V[i][j] = 0;
		}
	}

	srand(time(0));
	double *left_vector = (double*)malloc(M * sizeof(double));
	double *next_left_vector = (double*)malloc(M * sizeof(double));
	double *right_vector = (double*)malloc(N * sizeof(double));
	double *next_right_vector = (double*)malloc(N * sizeof(double));

	int col = 0;
	for (col = 0; col < K; col++) {
		double diff = 1;
		double r = -1;
		while (1)
		{
			for (i = 0; i < M; i++) {
				left_vector[i] = (float)rand() / RAND_MAX;
			}
			if (normalize(left_vector, M) > eps)
				break;
		}
		int iter = 0;
		for (iter = 0; diff >= eps && iter < MAX_ITER; iter++) {
			memset(next_left_vector, 0, sizeof(double)*M);
			memset(next_right_vector, 0, sizeof(double)*N);
			for (i = 0; i < M; i++)
				for (j = 0; j < N; j++)
					next_right_vector[j] += left_vector[i] * A[i][j];

			r = normalize(next_right_vector, N);

			if (r < eps)
				break;
			for (i = 0; i < col; i++)
				orth(&V[i][0], next_right_vector, N);
			normalize(next_right_vector, N);

			for (i = 0; i < M; i++)
				for (j = 0; j < N; j++)
					next_left_vector[i] += next_right_vector[j] * A[i][j];

			r = normalize(next_left_vector, M);

			if (r < eps)
				break;
			for (i = 0; i < col; i++)
				orth(&U[i][0], next_left_vector, M);

			normalize(next_left_vector, M);

			diff = 0;

			for (i = 0; i < M; i++)
			{
				double d = next_left_vector[i] - left_vector[i];
				diff += d*d;
			}

			memcpy(left_vector, next_left_vector, sizeof(double)*M);
			memcpy(right_vector, next_right_vector, sizeof(double)*N);
		}

		if (r >= eps) {
			S[col] = r;
			memcpy((char *)&U[col][0], left_vector, sizeof(double)*M);
			memcpy((char *)&V[col][0], right_vector, sizeof(double)*N);
		}
		else {
			break;
		}

	}
	free(next_left_vector);
	free(next_right_vector);
	free(left_vector);
	free(right_vector);
	next_left_vector = NULL;
	next_right_vector = NULL;
	left_vector = NULL;
	right_vector = NULL;
	return true;
}
#include "wavelet.h"

void branch_lp_dn(double *signal, int length, double *sigop)
{
	double *harrL = (double *)malloc(2 * sizeof(double));
	double *harrH = (double *)malloc(2 * sizeof(double));
	harrL[0] = 0.7071, harrL[1] = 0.7071;
	harrH[0] = 0.7071, harrH[1] = -0.7071;

	int temp_len = length;

	for (int i = 0; i < temp_len; i += 2)
	{
		sigop[i / 2] = signal[i] * harrL[0] + signal[i + 1] * harrL[1];
	}
	free(harrL);
	free(harrH);
	return;
}

void branch_hp_dn(double *signal, int length, double *sigop)
{
	double *harrL = (double *)malloc(2 * sizeof(double));
	double *harrH = (double *)malloc(2 * sizeof(double));
	harrL[0] = 0.7071, harrL[1] = 0.7071;
	harrH[0] = 0.7071, harrH[1] = -0.7071;

	int temp_len = length;

	for (int i = 0; i < temp_len; i += 2)
	{
		sigop[i / 2] = signal[i] * harrH[0] + signal[i + 1] * harrH[1];
	}
	free(harrL);
	free(harrH);
	return;
}

void branch_lp_hp_up(double *sigL, int lengthL, double *sigH, int lengthH, double *oup)
{
	if (lengthL != lengthH)
	{
		printf("error input.\n");
		return;
	}

	double *harrL = (double *)malloc(2 * sizeof(double));
	double *harrH = (double *)malloc(2 * sizeof(double));
	harrL[0] = 0.7071, harrL[1] = 0.7071;
	harrH[0] = 0.7071, harrH[1] = -0.7071;

	int temp_lenth = lengthL;
	for (int i = 0; i < temp_lenth; i++)
	{
		oup[2 * i] = (sigL[i] + sigH[i]) / (2 * harrL[0]);
		oup[2 * i + 1] = (sigL[i] - sigH[i]) / (2 * harrH[0]);
	}

	free(harrL);
	free(harrH);
	return;
}

void DWT2D(double **signal, int m, int n, double **cLL, double **cLH, double **cHL, double **cHH)
{
	if (m % 2 != 0 || n % 2 != 0)
	{
		printf("error input.\n");
		return;
	}
	int rows = m;
	int cols = n;
	int cols_lp1 = cols / 2;
	int cols_hp1 = cols / 2;

	double **lp_dn1 = (double**)malloc(rows * sizeof(double*));
	for (int i = 0; i < rows; i++)
	{
		lp_dn1[i] = (double*)malloc(cols_lp1 * sizeof(double));
	}
	// Implementing row filtering and column downsampling in each branch.
	for (int i = 0; i < rows; i++)
	{
		double *temp_row = (double *)malloc(cols * sizeof(double));
		for (int j = 0; j < cols; j++)
		{
			temp_row[j] = signal[i][j];
		}
		double *oup = (double *)malloc(cols_lp1 * sizeof(double));
		branch_lp_dn(temp_row, cols, oup);
		free(temp_row);
		temp_row = NULL;
		for (int j = 0; j < cols_lp1; j++)
		{
			lp_dn1[i][j] = oup[j];
		}
		free(oup);
		oup = NULL;
	}

	double **hp_dn1 = (double**)malloc(rows * sizeof(double*));
	for (int i = 0; i < rows; i++)
	{
		hp_dn1[i] = (double*)malloc(cols_hp1 * sizeof(double));
	}
	for (int i = 0; i < rows; i++)
	{
		double *temp_row2 = (double *)malloc(cols * sizeof(double));
		for (int j = 0; j < cols; j++) {
			temp_row2[j] = signal[i][j];
		}

		double *oup2 = (double *)malloc(cols_hp1 * sizeof(double));
		branch_hp_dn(temp_row2, cols, oup2);
		free(temp_row2);
		temp_row2 = NULL;
		for (int j = 0; j < cols_hp1; j++) {
			hp_dn1[i][j] = oup2[j];
		}
		free(oup2);
		oup2 = NULL;
	}

	cols = cols / 2;
	int temp_rows = rows / 2;
	// Implementing column filtering and row downsampling in Low Pass branch.
	for (int j = 0; j < cols; j++) {
		double *temp_row3 = (double*)malloc(rows * sizeof(double));
		//double *temp_row3 = new double[rows];
		for (int i = 0; i < rows; i++) {
			temp_row3[i] = lp_dn1[i][j];
		}
		//double *oup3 = new double[temp_rows];
		double *oup3 = (double*)malloc(temp_rows * sizeof(double));
		branch_lp_dn(temp_row3, rows, oup3);
		free(temp_row3);
		temp_row3 = NULL;
		for (int i = 0; i < temp_rows; i++) {
			cLL[i][j] = oup3[i];
		}
		free(oup3);
		oup3 = NULL;
	}

	for (int j = 0; j < cols; j++) {
		double *temp_row4 = (double*)malloc(rows * sizeof(double));
		for (int i = 0; i < rows; i++) {
			temp_row4[i] = lp_dn1[i][j];
		}
		double *oup4 = (double*)malloc(temp_rows * sizeof(double));
		branch_hp_dn(temp_row4, rows, oup4);
		free(temp_row4);
		temp_row4 = NULL;
		for (int i = 0; i < temp_rows; i++) {
			cLH[i][j] = oup4[i];
		}
		free(oup4);
		oup4 = NULL;
	}

	// Implementing column filtering and row downsampling in Low Pass branch.
	for (int j = 0; j < cols; j++) {
		//double *temp_row5 = new double[rows];
		double *temp_row5 = (double*)malloc(rows * sizeof(double));
		for (int i = 0; i < rows; i++) {
			temp_row5[i] = hp_dn1[i][j];
		}
		//double *oup5 = new double[temp_rows];
		double *oup5 = (double*)malloc(temp_rows * sizeof(double));
		branch_lp_dn(temp_row5, rows, oup5);
		free(temp_row5);
		temp_row5 = NULL;
		for (int i = 0; i < temp_rows; i++) {
			cHL[i][j] = oup5[i];
		}
		free(oup5);
		oup5 = NULL;
	}

	for (int j = 0; j < cols; j++) {
		//double *temp_row6 = new double[rows];
		double *temp_row6 = (double*)malloc(rows * sizeof(double));
		for (int i = 0; i < rows; i++) {
			temp_row6[i] = hp_dn1[i][j];
		}
		//double *oup6 = new double[temp_rows];
		double *oup6 = (double*)malloc(temp_rows * sizeof(double));
		branch_hp_dn(temp_row6, rows, oup6);
		free(temp_row6);
		temp_row6 = NULL;
		for (int i = 0; i < temp_rows; i++) {
			cHH[i][j] = oup6[i];
		}
		free(oup6);
		oup6 = NULL;
	}
	for (int i = 0; i < rows; i++) {
		free(hp_dn1[i]);
		free(lp_dn1[i]);
	}
	free(hp_dn1);
	free(lp_dn1);
	return;
}

void IDWT2D(double **signal, int LLm, int LLn, double **cLL, double **cLH, double **cHL, double **cHH)
{
	// Synthesis
	int cols = LLm;
	int rows = LLn;
	int rows_n = 2 * rows;
	// Row Upsampling and Column Filtering at the first LP Stage.
	double **cL = (double **)malloc(rows_n * sizeof(double *));
	for (int i = 0; i < rows_n; i++)
	{
		cL[i] = (double *)malloc(cols * sizeof(double));
	}
	double **cH = (double **)malloc(rows_n * sizeof(double *));
	for (int i = 0; i < rows_n; i++)
	{
		cH[i] = (double *)malloc(cols * sizeof(double));
	}

	for (int j = 0; j < cols; j++) {
		double *sigLL = (double*)malloc(rows * sizeof(double));
		double *sigLH = (double*)malloc(rows * sizeof(double));

		for (int i = 0; i < rows; i++) {
			sigLL[i] = cLL[i][j];
			sigLH[i] = cLH[i][j];
		}

		double *oup = (double*)malloc(rows_n * sizeof(double));
		branch_lp_hp_up(sigLL, rows, sigLH, rows, oup);
		free(sigLL);
		free(sigLH);
		sigLL = NULL;
		sigLH = NULL;
		for (int i = 0; i < rows_n; i++) {
			cL[i][j] = oup[i];
		}
		free(oup);
		oup = NULL;
	}

	for (int j = 0; j < cols; j++) {
		double *sigHL = (double*)malloc(rows * sizeof(double));
		double *sigHH = (double*)malloc(rows * sizeof(double));
		for (int i = 0; i < rows; i++) {
			sigHL[i] = cHL[i][j];
			sigHH[i] = cHH[i][j];
		}
		double *oup2 = (double*)malloc(rows_n * sizeof(double));
		branch_lp_hp_up(sigHL, rows, sigHH, rows, oup2);
		free(sigHL);
		free(sigHH);
		sigHL = NULL;
		sigHH = NULL;

		for (int i = 0; i < rows_n; i++) {
			cH[i][j] = oup2[i];
		}
		free(oup2);
		oup2 = NULL;
	}

	int cols_n = 2 * cols;
	for (int i = 0; i < rows_n; i++) {
		//double *sigL = new double[cols];
		//double *sigH = new double[cols];
		double *sigL = (double*)malloc(cols * sizeof(double));
		double *sigH = (double*)malloc(cols * sizeof(double));
		for (int j = 0; j < cols; j++) {
			sigL[j] = cL[i][j];
			sigH[j] = cH[i][j];
		}
		double *oup3 = (double*)malloc(cols_n * sizeof(double));
		branch_lp_hp_up(sigL, cols, sigH, cols, oup3);
		free(sigL);
		free(sigH);
		sigL = NULL;
		sigH = NULL;

		for (int j = 0; j < cols_n; j++) {
			signal[i][j] = oup3[j];
		}
		free(oup3);
		oup3 = NULL;
	}

	for (int i = 0; i < rows_n; i++)
	{
		free(cL[i]);
		free(cH[i]);
	}
	free(cL);
	cL = NULL;
	free(cH);
	cH = NULL;

	return;
}
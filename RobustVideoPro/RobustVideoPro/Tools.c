#include "Tools.h"

int Length(char *s)
{
	int i;
	for (i = 0; s[i] != '\0'; i++);
	return i;
}

void Itoa(int val, char* dst, int radix)
{
	char *_pdst = dst;
	if (!val)//允许val等于0 
	{
		*_pdst = '0';
		*++_pdst = '\0';
		return;
	}
	if (val < 0)
	{
		*_pdst++ = '-';
		val = -val;
	}
	char *_first = _pdst;
	char _cov;
	unsigned int _rem;
	while (val > 0)
	{
		_rem = (unsigned int)(val % radix);
		val /= radix;//每次计算一位 ，从低到高
		if (_rem > 9)//16进制
			*_pdst++ = (char)(_rem - 10 + 'a');
		else
			*_pdst++ = (char)(_rem + '0');
	}
	*_pdst-- = '\0';
	do { //由于数据是地位到高位储存的，需要转换位置
		_cov = *_pdst;
		*_pdst = *_first;
		*_first = _cov;
		_pdst--;
		_first++;
	} while (_first < _pdst);
}

void decToBin(int dec, char *output) {
	int result = 0, temp = dec, j = 1, sum = 0;
	if (temp == 0) {
		result = 0;
		sum = 1;
	}
	else {
		while (temp) {
			if (temp < 0) {
				temp = temp + 256;
			}
			result = result + j*(temp % 2);
			temp = temp / 2;
			j = j * 10;
			sum++;
		}
	}
	char *r = (char*)malloc((sum + 1) * sizeof(char));
	Itoa(result, r, 10);
	for (int i = sum - 1; i >= 0; i--) {
		output[8 - sum + i] = r[i];
	}
	free(r);
}

char BinTodec(int *bin) {
	int res = 0;
	int e = 1;
	for (int i = 7; i >= 0; --i) {
		res += bin[i] * e;
		e *= 2;
	}
	if (res > 127) {
		res = res - 256;
	}
	return (char)(res);
}

bool MatMul(double **a, int arow, int acol, double **b, int brow, int bcol, double **c)
{
	int am = arow;
	int an = acol;
	int bm = brow;
	int bn = bcol;

	for (int i = 1; i <= am; i++)
	{
		for (int j = 1; j <= bn; j++)
		{
			c[i][j] = 0;
			for (int k = 1; k <= bn; k++)
				c[i][j] += a[i][k] * b[k][j];
		}
	}
	return true;
}

/*****************************matMul() for svd-c*****************************/
bool matMul(double **a, int arow, int acol, double **b, int brow, int bcol, double **c)
{
	int am = arow;
	int an = acol;
	int bm = brow;
	int bn = bcol;

	for (int i = 0; i < am; i++)
	{
		for (int j = 0; j < bn; j++)
		{
			c[i][j] = 0;
			for (int k = 0; k < bn; k++)
				c[i][j] += a[i][k] * b[k][j];
		}
	}
	return true;
}
/****************************************************************************/

bool MatInv(double **a, int arow, int acol, double **output)
{
	int m = arow;
	int n = acol;
	for (int i = 1; i <= n; i++)
		for (int j = 1; j <= m; j++)
			output[i][j] = a[j][i];
	return true;
}

/*********************************matInv() for svd-c***********************************/
void matInv(double **a, int arow, int acol, double **output)
{
	int m = arow;
	int n = acol;
	for (int i = 0; i < n; i++)
		for (int j = 0; j < m; j++)
			output[i][j] = a[j][i];
	return;
}
/**************************************************************************************/

int randperm(int Num, int seed, int *array)
{
	int *temp = (int *)malloc(sizeof(int) * Num);
	int *flag = (int *)malloc(sizeof(int) * Num);
	for (int i = 0; i < Num; i++)
		flag[i] = 0;
	int step = (seed * seed) % Num;
	if (step < 2)
		step += 2;
	int pos = step;
	for (int i = 0; i < Num; ++i)
	{
		temp[i] = i;
	}
	for (int j = 0; j < Num; ++j) {
		while (flag[pos%Num]) {
			if (pos >= Num)
				pos = pos%Num;
			pos++;
		}
		array[j] = temp[pos%Num];
		flag[pos%Num] = 1;
		pos += step;
		if (step < 2)
			step += 2;
		step = (step + seed*seed) % Num;
	}
	free(temp);
	free(flag);
	return 0;
}

void padstring(char *s, int slength, int length, char *output)
{
	int len = (slength > length) ? slength : length;
	char *result = (char*)malloc(len * sizeof(char));
	if (slength < length)
	{
		for (int i = 0; i < length; i++)
		{
			if (i < length - slength)
			{
				result[i] = '0';
			}
			else result[i] = s[i - length + slength];
		}
	}
	else
	{
		for (int i = 0; i < slength; i++)
		{
			result[i] = s[i];
		}
	}
	for (int i = 0; i < len; i++) {
		output[i] = result[i];
	}
	free(result);
	return;
}
void padstring_front(char *s, int slength, int length, char *output)
{
	int len = (slength > length) ? slength : length;
	char *result = (char*)malloc(len * sizeof(char));
	if (slength < length)
	{
		for (int i = 0; i < length; i++)
		{
			if (i < length - slength)
			{
				result[i] = '0';
			}
			else
				result[i] = s[i - length + slength];
		}
	}
	else
	{
		for (int i = 0; i < slength; i++)
		{
			result[i] = s[i];
		}
	}
	for (int i = 0; i < len; i++) {
		output[i] = result[i];
	}
	free(result);
	return;
}
void padstring_behind(char *s, int slength, int length, char *output)
{
	int len = (slength > length) ? slength : length;
	char *result = (char*)malloc(len * sizeof(char));
	if (slength < length)
	{
		for (int i = 0; i < length; i++)
		{
			if (i >= slength)
			{
				result[i] = '0';
			}
			else
				result[i] = s[i];
		}
	}
	else
	{
		for (int i = 0; i < slength; i++)
		{
			result[i] = s[i];
		}
	}
	for (int i = 0; i < len; i++) {
		output[i] = result[i];
	}
	free(result);
	return;
}
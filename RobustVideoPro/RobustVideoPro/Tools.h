#pragma once
#ifndef TOOLS_H
#define TOOLS_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

int Length(char *s);
void Itoa(int val, char* dst, int radix);
void decToBin(int dec, char *output);
char BinTodec(int *bin);
bool MatMul(double **a, int arow, int acol, double **b, int brow, int bcol, double **c);
bool matMul(double **a, int arow, int acol, double **b, int brow, int bcol, double **c);
bool MatInv(double **a, int arow, int acol, double **output);
void matInv(double **a, int arow, int acol, double **output);
int randperm(int Num, int seed, int *array);
void padstring(char *s, int slength, int length, char *output);
void padstring_front(char *s, int slength, int length, char *output);
void padstring_behind(char *s, int slength, int length, char *output);
#endif // !TOOLS_H


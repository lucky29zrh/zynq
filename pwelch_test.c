matlab code
------
close all;
clear all;
%利用pwelch实现Welch法平均周期图平均法实现功率谱估计
Fs = 1024;
NFFT = 1024;
n = 0:1/Fs:1;
x = 2 * cos(2*pi*250*n) + 4*sin(2*pi*400*n);
window2 = hamming(256);
noverlap = 128;%数据重叠
[Pxx2,f2]=pwelch(x,window2,noverlap,NFFT,Fs);
% PXX2= 10*log10(Pxx2);
figure()
plot(f2,Pxx2);
title('汉明窗');


------
c语言：


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "Timing.h"
#include "CTools.h"
#include "CMath.h"
#include <stdio.h>
#include "includes.h"
/*
* fft.c
*
* Implementation of Fast Fourier Transform(FFT)
* and reversal Fast Fourier Transform(IFFT)
*
*  Created on: 2019-1-15
*      Author: wsqyouth
*/
#pragma warning(disable:4996)

#define PI 3.1415926

#if 0
void print2D(double arr[][4], int m, int n)
{
	for (int i = 0; i < m; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			cout << arr[i][j] << " ";
		}
		cout << endl;
	}

}

void print2DNew(double arr[][3], int m, int n)
{
	for (int i = 0; i < m; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			cout << arr[i][j] << " ";
		}
		cout << endl;
	}

}
#endif 

int main()
{
	const   int     signalLen = 1024;
	const   int     noverlap = 128; //每一段的重叠样本数
	const   int     nsc = 256;      //默认为nfft长度的海明窗Hamming大小 winsize=nfft
	double			Fs = 1024; 		//Sampling frequency
	complexType sinTable[signalLen] = { 0 };
	for (int n = 0; n < signalLen; ++n) {
		sinTable[n].Real = 2*cos(250 * 2 * PI*n / Fs);//?????ò???????????é?? (double)n
		sinTable[n].Imag = 4*sin(400 * 2 * PI*n / Fs);
	}
	double *result = (double*)malloc(nsc * sizeof(double));

	CTools  ctool;
	ctool.welchPSE(sinTable, signalLen, nsc, noverlap, result);
	for (int i = 0; i < nsc; ++i)
	{
		printf("%lf\n", result[i]);
	}


#if 1
	/***matlab plot-------start**/
	//步骤4：显示频谱
	Engine* eg = NULL;
	if (!(eg = engOpen(NULL)))
	{
		printf("Open matlab enging fail!");
		return 1;
	}

	const int dataNum = nsc;
	double xtemp[dataNum] = { 0 };
	double ytemp[dataNum] = { 0 };

	for (int i = 0; i < dataNum; i++)
	{
		//xtemp[i] = i * 2.0 * PI / dataNum;
		xtemp[i] = i*1.0 / dataNum * Fs;
		ytemp[i] = result[i]; //幅度

	}
	mat_plot(eg, xtemp, ytemp, dataNum, "-b", 1, 5);
	getchar();
	if (eg)
		engClose(eg);
	/***matlab plot-------end**/
#endif


	getchar();

	free(result);
	return 0;
}

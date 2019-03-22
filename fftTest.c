//该代码用来测试C语言中基于复数的fft,ifft,fftabs及文件操作功能。

matalab代码：
----
Fs = 200;            % Sampling frequency                    
T = 1/Fs;            % Sampling period       
N = 8192;             % Length of signal
t = (0:N-1)*T;       % Time vector
%cos参数为弧度，所以-30度和90度要分别换算成弧度。 
S=3*cos(2*pi*50*t)+1.5*cos(2*pi*75*t);  
figure()
plot(S);
title('原始信号');


X = fft(S,N); %FFT变换 重点关注 第1点、第51点、和第76点附近有较大值
AX = abs(X);
figure();
plot(AX);
title('FFT模值');

Y = abs(X);
Z = fftshift(Y);
figure();
subplot(2,1,1);%二行一列第一幅图
plot((-N/2:N/2-1)*Fs/N,Z);%实际频率 实际幅值 计算*2/N (直流分量其实不需要*2)
title('幅频特性曲线','fontsize',13);
xlabel('f/Hz','fontsize',13);%横坐标显示f/Hz 
ylabel('幅值','fontsize',13);%纵坐标显示幅值 




c code:
-----
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define  MM_PI (3.14159265358979323846264338327950288419716939937510)       // 圆周率atan(1)* 4

typedef double                    mathDouble;

/*定义复数类型*/
typedef struct _ReDefcomplex
{
	double    Real;
	double    Imag;
}complexType;


/*
*函数名：is_power_of_two
*说明：判定一个数是否是2的N次幂
*输入：
*输出：2的N次幂返回true
*/
bool is_power_of_two(int  num)
{
	if (num < 2)
		return false;
	if (num == 2)
		return true;

	return (num&(num - 1)) == 0;
}


/*
*函数名：BitReverse
*说明：变址计算，将x(n)码位倒置
*输入：复数数组x，大小size_x
*输出：
*/

void BitReverse(complexType x[], int size_x)
{
	complexType temp;
	unsigned short i = 0, j = 0, k = 0;
	double t;
	for (i = 0; i<size_x; i++)
	{
		k = i; j = 0;
		t = (log(size_x) / log(2));
		while ((t--)>0)    //利用按位与以及循环实现码位颠倒  
		{
			j = j << 1;
			j |= (k & 1);
			k = k >> 1;
		}
		if (j>i)			  //将x(n)的码位互换  
		{
			temp = x[i];
			x[i] = x[j];
			x[j] = temp;
		}
	}

}


/*
* 函数名：Cfft
* 说  明：FFT Algorithm
* === Inputs ===
* inVec:   complex numbers
* vecLen : nodes of FFT. @N should be power of 2, that is 2^(*)
* === Output ===
* outVec: 独立输出,不影响输入
* 其他：函数返回值false表示非法参数
*/
bool Cfft(complexType  const inVec[], int  const vecLen, complexType  outVec[])           //fft
{
	if ((vecLen <= 0) || (NULL == inVec) || (NULL == outVec))
		return false;
	if (!is_power_of_two(vecLen))
		return false;

	//步骤1 初始化因子数组Weights
	complexType  *Weights = (complexType *)malloc(sizeof(complexType) * vecLen); //权重数组
																				 // 计算权重序列
	double fixed_factor = (-2 * MM_PI) / vecLen;
	for (int i = 0; i < vecLen / 2; i++) {
		double angle = i * fixed_factor;
		Weights[i].Real = cos(angle);
		Weights[i].Imag = sin(angle);
	}
	for (int i = vecLen / 2; i < vecLen; i++) {
		Weights[i].Real = -(Weights[i - vecLen / 2].Real);
		Weights[i].Imag = -(Weights[i - vecLen / 2].Imag);
	}

	//步骤2:目的数组清零,源数组做change变址运算后拷贝目的数组
	memset(outVec, 0, vecLen * sizeof(complexType));

	//计算倒序位码
	int r = (int)(log(vecLen) / log(2));//2的多少次幂
	int index = 0;
	for (int i = 0; i < vecLen; i++) {
		index = 0;
		for (int m = r - 1; m >= 0; m--) {
			index += (1 && (i & (1 << m))) << (r - m - 1);
		}
		outVec[i].Real = inVec[index].Real;
		outVec[i].Imag = inVec[index].Imag;
	}

	/*for (int i = 0; i < vecLen; ++i)
	{
	printf("%lf %lf\n", outVec[i].Real, outVec[i].Imag);
	}*/

	// 计算快速傅里叶变换对（变地址后端outVec蝶形运算）
	int i = 0, j = 0, k = 0, m = 0;
	complexType up, down, product;
	int size_x = vecLen;
	for (i = 0; i< log(size_x) / log(2); i++) /*一级蝶形运算 stage */
	{
		m = 1 << i;
		for (j = 0; j<size_x; j += 2 * m)     /*一组蝶形运算 group,每组group的蝶形因子乘数不同*/
		{
			for (k = 0; k<m; k++)             /*一个蝶形运算 每个group内的蝶形运算的蝶形因子乘数成规律变化*/
			{
				//优化，减少函数调用
				//complexMultiply(outVec[j + k + l], Weights[size_x*k / 2 / l], &product);
				product.Real = outVec[j + k + m].Real *  Weights[size_x*k / 2 / m].Real - outVec[j + k + m].Imag *  Weights[size_x*k / 2 / m].Imag;
				product.Imag = outVec[j + k + m].Imag *  Weights[size_x*k / 2 / m].Real + outVec[j + k + m].Real *  Weights[size_x*k / 2 / m].Imag;
				//complexAdd(outVec[j + k], product, &up);
				up.Real = outVec[j + k].Real + product.Real;
				up.Imag = outVec[j + k].Imag + product.Imag;
				//complexSubtract(outVec[j + k], product, &down);
				down.Real = outVec[j + k].Real - product.Real;
				down.Imag = outVec[j + k].Imag - product.Imag;
				outVec[j + k] = up;
				outVec[j + k + m] = down;
			}
		}
	}

	//步骤3: 释放所占空间
	free(Weights);

	return true;
}

/*
* 函数名：Cifft
* 说  明：Inverse FFT Algorithm
* === Inputs ===
* inVec:   complex numbers
* vecLen : nodes of FFT. @N should be power of 2, that is 2^(*)
* === Output ===
* outVec: 独立输出,不影响输入
* 其他：ifft本质上调用fft，只是在之前之后进行适当操作即可
*       该代码结构清晰，稍微耗时
*/
bool  Cifft(complexType  const inVec[], int  const vecLen, complexType  outVec[])            //ifft
{
	complexType* in_Vec_New = (complexType*)malloc(vecLen * sizeof(complexType));
	//输入数据虚部取反写入in_Vec_New
	for (int i = 0; i < vecLen; i++) {
		in_Vec_New[i].Real = inVec[i].Real;
		in_Vec_New[i].Imag = -inVec[i].Imag;
	}
	//fft变换
	Cfft(in_Vec_New, vecLen, outVec);

	//结果/N
	for (int k = 0; k <= vecLen - 1; k++) {
		outVec[k].Real = outVec[k].Real / vecLen;
		outVec[k].Imag = -outVec[k].Imag / vecLen;
	}

	if (in_Vec_New) {
		free(in_Vec_New);
	}
	return true;
}

/*函数名：Rfftshift
*说明：将实数类型double in数组本地进行shift(in place)
*输入：待变换数组和长度
*输出：
*其它：空间复杂度O(n),时间复杂度O(n)
*/
void  Rfftshift(double *in, int dim)
{
	//in is even
	if ((dim & 1) == 0) {
		int pivot = dim / 2;
		for (int pstart = 0; pstart<pivot; ++pstart) {//swap two elements
			double temp = in[pstart + pivot];
			in[pstart + pivot] = in[pstart];
			in[pstart] = temp;
		}
	}
	else { //in is odd
		int pivot = dim / 2;
		double pivotElement = in[pivot];
		for (int pstart = 0; pstart<pivot; ++pstart) {//swap two elements
			double temp = in[pstart + pivot + 1];
			in[pstart + pivot] = in[pstart];
			in[pstart] = temp; //将pivot+1对应的元素与pstart对应元素交换
		}
		in[dim - 1] = pivotElement;
	}
}


/*函数名：fftshift
*说明：将in数组本地进行shift(in place)
*输入：待变换数组和长度
*输出：
*其它：空间复杂度O(n),时间复杂度O(n)
*/
void  Cfftshift(complexType *in, int dim)      //shift
{
	//in is even
	if ((dim & 1) == 0) {
		int pivot = dim / 2;
		for (int pstart = 0; pstart<pivot; ++pstart) {//swap two elements
			complexType temp = in[pstart + pivot];
			in[pstart + pivot] = in[pstart];
			in[pstart] = temp;
		}
	}
	else { //in is odd
		int pivot = dim / 2;
		complexType pivotElement = in[pivot];
		for (int pstart = 0; pstart<pivot; ++pstart) {//swap two elements
			complexType temp = in[pstart + pivot + 1];
			in[pstart + pivot] = in[pstart];
			in[pstart] = temp; //将pivot+1对应的元素与pstart对应元素交换
		}
		in[dim - 1] = pivotElement;
	}
}


/*函数名：CfftAbs
*说明：对复数类型complexType求模运算
*输入：待复数类型数组和长度
*输出：abs求幅度
*其它：空间复杂度O(n),时间复杂度O(n)
*/
void  CfftAbs(complexType x[], int N, double *amplitude) // 求模运算
{
	for (int i = 0; i < N; i++)
	{
		//amplitude[i] = 20.0 * log10( qSqrt( g.real() * g.real() + g.imag() * g.imag() ) );
		//实际幅值 计算*2/N (直流分量其实不需要*2)
		//if(i !=0)
		amplitude[i] = sqrt(x[i].Real*x[i].Real + x[i].Imag*x[i].Imag);
	}
}

/*函数名：Coutput
*说明：输出傅里叶变换的结果,方便测试
*输入：待打印数组和长度
*输出：
*/
void  Coutput(const complexType x[], int size_x)         //输出傅里叶变换的结果
{
	int i;
	printf("The result are as follows：\n");
	for (i = 0; i<size_x; i++)
	{
		printf("%4d  %8.4f ", i, x[i].Real);
		if (x[i].Imag >= 0.0001)printf(" +%8.4fj\n", x[i].Imag);
		else if (fabs(x[i].Imag)<0.0001) printf("\n");
		else printf("  %8.4fj\n", x[i].Imag);
	}
}
/*函数名：Routput
*说明：输出傅里叶变换的结果,方便测试
*输入：待打印数组和长度
*输出：
*/
void  Routput(const mathDouble x[], int size_x)         //输出傅里叶变换的结果
{
	int i;
	printf("The result are as follows：\n");
	for (i = 0; i<size_x; i++)
	{
		printf("%4d  %8.4f\n", i, x[i]);
	}
}

void printArr2D(double arr[][2], int N)     //打印二维数组
{
	for (int i = 0; i<N; ++i)
	{
		printf("%lf %lf\n", arr[i][0], arr[i][1]);
	}
	printf("data has print\n");
}
void writefileArr2D(const char* filename, double arr[][2], int rank) //写文件
{
	FILE *fp;
	if ((fp = fopen(filename, "w")) == NULL)
	{
		printf("file open error");
	}
	for (int i = 0; i<rank; ++i)
	{
		fprintf(fp, "%lf,%lf\n", arr[i][0], arr[i][1]);
	}
	fclose(fp);


}
void writefileArr(const char* filename, double arr[], int rank) //写文件
{
	FILE *fp;
	if ((fp = fopen(filename, "w")) == NULL)
	{
		printf("file open error");
	}
	for (int i = 0; i<rank; ++i)
	{
		fprintf(fp, "%lf\n", arr[i]);
	}
	fclose(fp);


}


void CreadfileArr2D(const char* filename, complexType* in_Vec, int rank) //读文件
{
	FILE *fp;
	if ((fp = fopen(filename, "r")) == NULL)
	{
		printf("file not exist\n");
	}
	for (int i = 0; i<rank; ++i)
	{
		fscanf(fp, "%lf,%lf\n", &(in_Vec[i].Real), &(in_Vec[i].Imag));
	}
	fclose(fp);
}

void testfft()
{
        const int N = 32;
	
        double yAbs[N];
	
        complexType x[N];	   /*输入序列*/
        complexType y[N];	   /*输出序列*/
        complexType z[N];	   /*输入序列*/
	
        //printf(" 输出DIT方法实现的FFT结果\n");

        /* Init inputs */
        //for (int i = 0; i < N; i++) {
            //x[i].Real = (double)i;//输入序列的实部和虚部
            //x[i].Imag = 0.0;
            //x[i].real = sin(2*PI*i/N);  //正弦波
            //x[i].imag = 0;

        //}
      double Fs = 200;
      for (int i = 0; i < N; i++)
      {
        x[i].Real = (double)3 * sin(2 * MM_PI* 50 * i /Fs) + 1.5 * cos(2 * MM_PI * 75 * i / Fs);
        x[i].Imag = 0.0;
      }

        Coutput(x,N); //调用输出输入序列
        Cfft(x, N, y);  //调用快速傅里叶变换
        Coutput(y, N);//调用输出傅里叶变换结果函数
        Cifft(y, N, z);
        Coutput(z, N);

        CfftAbs(y,N,yAbs);

        Rfftshift(yAbs,N);
        Routput(yAbs,N);
	
        //writefileArr("ffttest1.txt",mozhi,N);
}


int main()
{
	testfft();
	return 0;

}




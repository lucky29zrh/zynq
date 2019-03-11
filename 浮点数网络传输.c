//浮点数的存储不能采用类似u32的取整方法，需要直接使用memcpy保留精度。
参考文章:https://www.cnblogs.com/MikeZhang/p/floatNetworkTrans20180320.html

#include <math.h>
#include <stdio.h>
#include <string.h>



#include <stdio.h>
#include <string.h>

typedef char                s8;
typedef unsigned char       u8;
typedef short               s16;
typedef unsigned short      u16;
typedef int                 s32;
typedef unsigned    int     u32;
typedef float               f32;
typedef signed long long    s64;
typedef unsigned long long  u64;


typedef struct
{
    u16 head;        //帧头２字节
    u32 len;         //整包长度，４字节
    u32 temperature; //温度 4字节
    u32 humidity;    //湿度 4字节
    u16 tail;       //帧尾２字节
}__attribute__ ((packed)) DataFrame;

typedef struct
{
    float temperature ;
    float humidity    ;
}__attribute__ ((packed)) DataBoard;

void printArr(char buf[],int n)
{
    for(int j=0;j<n;j++)
    {
        printf("%x\n",buf[j]);
    }
}


unsigned char DealFrameDataSend(char *buf, DataBoard *smallData)
{
     int index = 0;
    //数据帧头
    //0 1
    *(u16*)(&buf[index]) = 0X7E7E;
     index+=2;
    //2 3 4 5
    *(u32*)(&buf[index]) = 0;
    index+=4;
 	memcpy((&buf[index]),&(smallData->temperature),4);
    index +=4;
	memcpy((&buf[index]),&(smallData->humidity),4);
    index +=4;
    
    //数据帧尾
     *(u16*)(&buf[index]) = 0XA5A5;
    index+=2;
    
    //填充内容实际长度
    *(u32*)(&buf[2]) = index-6;//update content len
    
    return index;
}
void DealFrameDataRecv(void *data, DataBoard *conData)
{
 	printArr(data,10);
	int index = 0;
	conData->temperature = *(float *)(&data[index]);
	index += 4;
	conData->humidity = *(float *)(&data[index]);
	index += 4;


	printf("%f\n", conData->temperature);
	printf("%f\n", conData->humidity); //
}

int main() {
    
   //封装成帧发出去
    s8 buf[256] = {0};
    float temperature = 9.46;
    float humidity    = -3.14;
    DataBoard cData;
    cData.temperature = temperature;
    cData.humidity =  humidity;
    int newLen = DealFrameDataSend(buf,&cData);
    printArr(buf,newLen);
    
    //
    DataBoard recvData;
    s8 recv_buf[256] = {0};
    int content_len =  *(u32*)(&buf[2]);
    memcpy(recv_buf,(void*)(&buf[6]),content_len);
    DealFrameDataRecv(recv_buf, &recvData);
    
   // unsigned char *q=(unsigned char *)&value;
   // float ans = *(float*)q;
  //  cout<<ans<<endl;
	return 0;
}

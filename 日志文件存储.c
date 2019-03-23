功能实现：
每次运行以当前日期作为文件名
每个文件通过追加的方式，对数据进行不超过10行的文件存储，若文件超过10行上限，则直接退出

--
version1:此时数据以自己的具体类型存储、读写
----
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

void generateFilename(char *filename,int size)
{
	time_t t = time(NULL);
    strftime(filename, sizeof(char)*size, "%m%d_%H_%M_%S.txt",localtime(&t) );
	//printf("%s\n",filename);
}
void wirteFileArr1D(const char* filename,uint32_t rxLoFrequency,int16_t idata,int16_t qdata)
{
	  static unsigned long index = 0;

	  if(index >= 10)
	  {
		  printf("the file lines exceed 1000000\n");;
		  return ;
	  }
  	  FILE * fp;
  	  if((fp = fopen(filename, "a+"))==NULL)
	  {
		  printf("cant open the file");
		  return;
	  }
	
      fprintf(fp,"%d,%d,%d\n",rxLoFrequency,idata,qdata);
	  index++;
	  //printf("%lu\n",index);
	 
	  fclose (fp);
}

void readFileArr1D(const char* filename,int size)
{
    FILE * fp;
    if((fp=fopen(filename,"r"))==NULL)
    {
      printf("cant open the file");
	  return;
    }
	uint32_t rxLoFrequency;
    int16_t idata ;
    int16_t qdata ;

	for(int i=0;i<size;++i)
	{
		fscanf(fp,"%d,%d,%d\n",&rxLoFrequency,&idata,&qdata);
		printf("%d,%d,%d\n",rxLoFrequency,idata,qdata);
	}
   
   

}
int main ()
{
	uint32_t rxLoFrequency = 2140000000;
    int16_t idata = -56;
    int16_t qdata = -19;
	
	
	char filename[100] = {0};
 	generateFilename(filename,100);
	
	for(int i=0;i<12;++i)
  	wirteFileArr1D(filename,rxLoFrequency,idata,qdata);
    
	readFileArr1D(filename,12);
	

    return 0;
}


----
Version2: 数据统统以doule型存储、读写
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

void generateFilename(char *filename,int size)
{
	time_t t = time(NULL);
    strftime(filename, sizeof(char)*size, "%m%d_%H_%M_%S.txt",localtime(&t) );
	//printf("%s\n",filename);
}
void wirteFileArr1D(const char* filename,uint32_t rxLoFrequency,int16_t idata,int16_t qdata)
{
	  static unsigned long index = 0;

	  if(index >= 10)
	  {
		  printf("the file lines exceed 1000000\n");;
		  return ;
	  }
  	  FILE * fp;
  	  if((fp = fopen(filename, "a+"))==NULL)
	  {
		  printf("cant open the file");
		  return;
	  }
	
      fprintf(fp,"%lf,%lf,%lf\n",(double)rxLoFrequency,(double)idata,(double)qdata);
	  index++;
	  //printf("%lu\n",index);
	 
	  fclose (fp);
}

void readFileArr1D(const char* filename,int size)
{
    FILE * fp;
    if((fp=fopen(filename,"r"))==NULL)
    {
      printf("cant open the file");
	  return;
    }
	double rxLoFrequency;
    double idata ;
    double qdata ;

	for(int i=0;i<size;++i)
	{
		fscanf(fp,"%lf,%lf,%lf\n",&rxLoFrequency,&idata,&qdata);
		printf("%lf,%lf,%lf\n",rxLoFrequency,idata,qdata);
	}
   
   

}
int main ()
{
	uint32_t rxLoFrequency = 2140000000;
    int16_t idata = -56;
    int16_t qdata = -19;
	
	
	char filename[100] = {0};
 	generateFilename(filename,100);
	
	for(int i=0;i<12;++i)
  	wirteFileArr1D(filename,rxLoFrequency,idata,qdata);
    
	readFileArr1D(filename,12);
	

    return 0;
}


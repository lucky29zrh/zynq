//这对TCP模型主要是用来测试I,Q数据信号的

---
//tcpServer.cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

int32_t listenfd, connectfd;     //socket描述符

void  tcp_server(uint16_t port)
{
  
    //调用socket，创建监听客户端的socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Creating socket failed.");
        exit(1);
    }
    
    //设置socket属性，端口可以重用
    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    //初始化服务器地址结构体
	struct sockaddr_in my_addr;
    bzero(&my_addr,sizeof(my_addr));
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(port);
    my_addr.sin_addr.s_addr = htonl (INADDR_ANY);

    //调用bind，绑定地址和端口
    if (bind(listenfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("Bind error.");
        exit(1);
    }

    //调用listen，开始监听
    if(listen(listenfd, 5) == -1){
        perror("listen() error\n");
        exit(1);
    }

}

void decompose(int32_t  buf[], int n)
{
    printf("32-bit Word: I : Q\n");
    for(int j=0;j<n;j++)
    {
         int32_t val = buf[j];
         char *p_dat=(char *)&val;
         const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
	 const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
	 printf("0x%.8X : %8d : %8d\n",val,i,q);
    }
}

int main()
{
	tcp_server(8000);
    
    struct sockaddr_in client;     //客户端地址信息结构体
    //调用accept，返回与服务器连接的客户端描述符
	uint32_t sin_size = sizeof(struct sockaddr_in);
    char buf[1024]; 
	if ((connectfd = accept(listenfd,(struct sockaddr *)&client, (socklen_t *)&sin_size)) == -1) {
            perror("accept() error\n");
    }else {  
            printf("connected with ip: %s  and port: %d\n", inet_ntop(AF_INET,&client.sin_addr, buf, 1024), ntohs(client.sin_port));  
    }  
	int num;
    int8_t recvbuf[5000];
	int32_t content_buf[3000];
	int count = 0;
	
	int ret;
    u8* data;
	u8 recv_buf[6]; //包头+数据长度
    //MSG_WAITALL
 	while (1) {
		memset(recvbuf, '\0', sizeof(recv_buf)); 
		ret=recv(connectfd, recv_buf, sizeof(recv_buf),0);// 接收数据
	    if(ret>0)
	    {

		   //printf("recv buf len:%d----------------%x  \r\n",ret,*(u8*)(&recv_buf[0]));
		   for(int i=0;i<ret;i++)
		   {
			  printf("%x  ",*(u8*)(&recv_buf[i]));
		   } 
		   printf("\r\n");
		   if(*(u16*)(&recv_buf[0])==0x7e7e)
		   {
			   printf("head = %x\r\n",*(u16*)(&recv_buf[0]));
			   u32 packet_len = *(u32*)(&recv_buf[2]); //content len + tail len
			   data = malloc(packet_len);  //开辟实际长度的data
			   printf("len = %d \r\n",packet_len);
			   ret=recv(connectfd, data,packet_len, 0);// 接收数
			   if(ret>0)
			   {	
					printf("ret = %d \r\n",ret);   
					/*
			   		   printf("ret ,data len%d************-\r\n",ret);
					   //content + tail 
			   		   for(int i=0;i<ret;i++)
			   		   {
			   			   printf("%x\r\n",*(u8*)(&data[i]));
			   		   }
			   		   printf("**************\r\n");
					*/
				printf("content+tail len%d,%0X  %0X\r\n",ret,*(u8*)(&data[ret-2]),*(u8*)(&data[ret-1]));
			  }
		  }
		}
/*
		num = 
		
		if(num > 0)
		{
			printf("num=%d\n",num);
			//decompose(recvbuf,num);
			for(int i=0;i<num;i+=4)
			{
				//printf("0x%.8X \n",recvbuf[i]);				
				content_buf[count]=*(int32_t*)(&recvbuf[i]);
				//printf("0x%.8X \n",content_buf[count]);
				count++;
			}
			printf("count=%d\n",count);	
			decompose(content_buf,count);
		}
*/		
 	}
	return 0;
}


---
//tcpClient.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

int16_t sockfd;
int16_t err_log;

int Tcp_Client_Init(uint8_t* ip,uint16_t port)
{
	uint16_t server_port = port;   // 服务器的端口号6001
	uint8_t  *server_ip =  ip; // 服务器ip地址"192.168.33.33"
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);      // 创建通信端点：套接字
	if (sockfd < 0) {
		perror("socket failed");
		//exit(-1);
		return -1;
	}
	
    //初始化服务器地址结构体
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));    // 初始化服务器地址
	server_addr.sin_family = AF_INET;   		 // IPv4
	server_addr.sin_port = htons(server_port); // 端口 6001
	inet_pton(AF_INET, server_ip, &server_addr.sin_addr.s_addr);    // ip
	// 主动连接服务器
	err_log = connect(sockfd, (struct sockaddr*) &server_addr,sizeof(server_addr));
	if (err_log != 0) {
		printf("connect(%s) failure!\n",server_ip);
		//close(sockfd);
		//exit(-1);
		return -1;
	}else
		printf("connected %s \n",server_ip);

	return 0;
}

int MySend(int iSock,char *pchBuf,size_t iLen){
	int iThisSend;
	unsigned int iSended=0;//has send bytes
	if(iLen == 0)
		return (0);
	while(iSended<iLen){
		do{
			iThisSend = send(iSock,pchBuf,iLen,0);//this time  
		}while( (iThisSend<0) && (errno==EINTR));
		if(iThisSend <0){
			return (iSended);
		}
		iSended += iThisSend;
		pchBuf += iThisSend;
	}
	return (iLen);
}

int main()
{
	int ret;
    /**TCP 客户端初始化**/
	ret = Tcp_Client_Init("127.0.0.1",8000);
	if (ret != 0) {
		printf("127.0.0.1,tcp client init failed!\n");
	}
    
    //FF 7F 00 00
	int32_t content_buf[1024] = {0x00007FFF,0x278D79BB,0x4B3B678D,0x678D4B3B,0x79BB278D,0x7FFF0000,0x79BBD873,
                     	  0x678DB4C5,0x4B3B9873,0x278D8645,0x00008001,0xD8738645,0xB4C59873,0x8645D873,
                          0x80010000,0x8645278D,0x98734B3B,0xB4C5678D,0xD87379BB};
	
	u8 *pSendBuf;
	pSendBuf = malloc(5000);  //开辟SendBuf
	int iLen = 0;

	int contenLen = sizeof(content_buf);
	
    //2+4+ len=79 +2	
    //7E7E  len  content_buf 0D0A
	pSendBuf[0] = 0x7e; pSendBuf[1] = 0x7e;   //7E 7E 
	//整包长度，４字节 pSendBuf[2]  pSendBuf[3] pSendBuf[4] pSendBuf[5]
	iLen += 6; //head + len
    memcpy(pSendBuf+iLen,content_buf, contenLen);
	iLen += contenLen;
	*(u16*)(&pSendBuf[iLen]) = 0x0a0d; //帧尾 0D 0A
	iLen += 2;
	*(u32*)(&pSendBuf[2])= contenLen+2;//４字节整包长度，填充实际长度
	printf("content+tail len:%d\n",contenLen+2); //
	printf("total len:%d\n",iLen);
while(1)
{	
	if(err_log == 0)
	{
		 int nbytes=send(sockfd, pSendBuf, iLen, 0);
		 //int nbytes=MySend(sockfd,pSendBuf,iLen);
		 printf("sent  %d,errno=%d\n",nbytes,errno);
		
		 //sleep(1);
	}

}

	return 0;
}

----
//makefile
all:tcpServer tcpClient

tcpServer:tcpServer.o

	gcc -g -o tcpServer tcpServer.o

tcpClient:tcpClient.o

	gcc -g -o tcpClient tcpClient.o

tcpServer.o:tcpServer.c

	gcc -g -c tcpServer.c

tcpClient.o:tcpClient.c

	gcc -g -c tcpClient.c

clean:all

	rm all
  

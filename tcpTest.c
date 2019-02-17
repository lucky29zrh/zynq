//tcpServer.c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>


int  TCPServerInit(uint16_t port,uint16_t backlog)
{
  	int32_t listenfd=-1;

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
    if(listen(listenfd, backlog) == -1){
        perror("listen() error\n");
        exit(1);
    }

	return listenfd;
}


int main()
{
	int32_t listenfd=TCPServerInit(8000,5);//端口为8000，连接队列为5
    int32_t connectfd;     				   //socket描述符
    
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
    int8_t recvbuf[1024];

	int count = 0;

    //MSG_WAITALL
 	while (1) {
		memset(recvbuf, '\0', sizeof(recvbuf));  
		num = recv(connectfd, recvbuf, sizeof(recvbuf),0);
		
		if(num > 0)
		{
			printf("receive num=%d\n",num);
			printf("------------------------\n");
		}
		
 	}
	return 0;
}


//tcpClient.c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include <pthread.h>

int16_t sockfd;
int16_t err_log;

int Tcp_Client_Init(uint8_t* ip,uint16_t port)
{
	uint16_t server_port = port;   // 服务器的端口号6001
	uint8_t  *server_ip =  ip; // 服务器ip地址"192.168.33.33"
	
	//printf("connect %s ...\n",server_ip);
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
		printf("connect %s failed!\n",server_ip);
		//close(sockfd);
		//exit(-1);
		return -1;
	}else
		printf("connected %s \n",server_ip);

	return 0;
}



int main()
{
	int16_t ret;
    /**TCP 客户端初始化**/
	ret = Tcp_Client_Init("127.0.0.1",8000);
	if (ret != 0) {
		printf("127.0.0.1,tcp client init failed!\n");
	}
    printf("start send\n");
	char send_buf[] ="hello";
while(1)
{	
	if(err_log == 0)
	{
		 //MSG_NOSIGNAL:Don't generate a SIGPIPE signal
		 //MSG_DONTWAIT: Enables nonblocking operation; if the operation would block, EAGAIN or EWOULDBLOCK is returned.
		 int ret =send(sockfd, send_buf, sizeof(send_buf), MSG_DONTWAIT);
  		 if (ret < 0)
		 {
				printf("send error%d  %d\n",ret,errno);
				
		 }else
		 {
			 printf("sent  %d\n",ret);
		 }
		
	}
	sleep(1);
}
	return 0;
}


------
//封装为socket.c
#include "includes.h"

////////////////////////////////////////////////////////////////////////////
//					UDP   Funchtion								          //
////////////////////////////////////////////////////////////////////////////

/*
 * @brief		initialize UDP server
 * @port		port number for socket
 * @serverfd	server socket fd
 * return server socked fd for success, on error return error code
 * */
int	UDPServerInit(int port, int *serverfd) {
	struct sockaddr_in server;
	// create socket , same as client
	*serverfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(*serverfd < 0) return -1;
	/// initialize structure dest
	memset((void*)&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;
	// Assign a port number to socket
	bind(*serverfd, (struct sockaddr*)&server, sizeof(server));

	return *serverfd;
}
/*
 * @brief	initialize UDP client
 * @clientfd	client socket fd
 * return client socked fd for success, on error return error code
 */
int	UDPClientInit(int *clientfd) {
	*clientfd = socket(AF_INET, SOCK_DGRAM, 0);

	return *clientfd;
}
/*
 * @brief		UDPRecv from UDP socket
 * @clientfd	socket fd
 * @buf	     	receving buffer
 * @size		buffer size
 * @from_ip		the client ip address of requested from UDP
 * return	    the length of receive data
 */
int	UDPRecv(int sockfd, void *buf, size_t size, char *from_ip, int *port) {
	struct sockaddr_in client;
	size_t addrlen = 0;
	int rc;
	memset(&client, '\0', sizeof(client));
	addrlen = sizeof(client);
	rc = recvfrom(sockfd, buf, size, 0, (struct sockaddr *)&client, (socklen_t *)&addrlen);
	strcpy(from_ip, (const char *)inet_ntoa(client.sin_addr));
	*port = htons(client.sin_port);

	return rc;
	
}


int UDPMyRecv(int sockfd, void *buf, size_t size){
	struct sockaddr_in client;
	size_t addrlen = 0;
	int rc;
	memset(&client, '\0', sizeof(client));
	addrlen = sizeof(client);
	rc = recvfrom(sockfd, buf, size, 0, (struct sockaddr *)&client, (socklen_t *)&addrlen);
	return rc;
}
/*
 * @brief		UDPSend from UDP socket
 * @clientfd	socket fd
 * @buf	     	sending buffer
 * @size		buffer size
 * @to_ip		the ip address of target server
 * return	    the length of sending data
 */
int	UDPSend(int sockfd, const void *buf, size_t size, const char *to_ip, const int *port) {
	struct sockaddr_in server;
	memset(&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(*port);
	inet_aton(to_ip, &server.sin_addr);

	return sendto(sockfd, buf, size, 0, (struct sockaddr *)&server, sizeof(server));
}
/*
 * close the socket
 * */
int UDPClose(int sockfd) {
	return close(sockfd);
}




////////////////////////////////////////////////////////////////////////////
//					TCP   Funchtion								          //
////////////////////////////////////////////////////////////////////////////

int  TCPServerInit(uint16_t port,uint16_t backlog)
{
  	int32_t listenfd=-1;

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
    if(listen(listenfd, backlog) == -1){
        perror("listen() error\n");
        exit(1);
    }

	return listenfd;
}

int TCPClientInit(uint8_t* ip,uint16_t port)
{
	uint16_t server_port = port;   // 服务器的端口号6001
	uint8_t  *server_ip =  ip; // 服务器ip地址"192.168.33.33"
	
	//printf("connect %s ...\n",server_ip);
	int clientfd = socket(AF_INET, SOCK_STREAM, 0);      // 创建通信端点：套接字
	if (clientfd < 0) {
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
	int err_log = connect(clientfd, (struct sockaddr*) &server_addr,sizeof(server_addr));
	if (err_log < 0) {
		printf("connect %s failed!\n",server_ip);
		//close(clientfd);
		//exit(-1);
		return -1;
	}else
		printf("connected %s \n",server_ip);

	return clientfd;
}


int TCPMySend(int iSock,char *pchBuf,size_t iLen){
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


int TCPMyRecv(int iSock,char * pchBuf,size_t tCount){

	 size_t tBytesRead=0;
        int iThisRead;
        while(tBytesRead < tCount){
              do{
                     iThisRead = read(iSock, pchBuf, tCount-tBytesRead);
              } while((iThisRead<0) && (errno==EINTR));
              if(iThisRead < 0){
                      return(iThisRead);
              }else if (iThisRead == 0)
                      return(tBytesRead);
              tBytesRead += iThisRead;
              pchBuf += iThisRead;
       }
}



////////////////////////////////////////////////////////////////////////////
//					dataCheck   Funchtion								  //
////////////////////////////////////////////////////////////////////////////
#define CRC_16_POLYNOMIALS   0x8005
/**
 * Calculating CRC-16 in 'C'
 * @para pchMsg, start of data
 * @para wDataLen, length of data
 * @para crc, incoming CRC
 * CRC16,校验和位宽W=16, 生成多项式x16+x15+x2+1
 * 除数（多项式）0x8005,余数初始值0x0000,结果异或值0x0000
 */
uint16_t crc16_caculate(uint8_t *pchMsg, uint16_t wDataLen) {
    uint8_t i;
    uint8_t chChar;
    uint16_t wCRC = 0xFFFF; //g_CRC_value;

    while (wDataLen--) {                     /* Step through bytes in memory */
        chChar = *pchMsg++;
        wCRC ^= (((uint16_t) chChar) << 8);  /* Fetch byte from memory, XOR into CRC top byte*/

        for (i = 0; i < 8; i++) {            /* Prepare to rotate 8 bits */
            if (wCRC & 0x8000) {             /* b15 is set... */
                wCRC = (wCRC << 1) ^ CRC_16_POLYNOMIALS;   /* rotate and XOR with polynomic */
            } else {                         /* just rotate */
                wCRC <<= 1;                  /* Loop for 8 bits */
            }
        }
        
        wCRC &= 0xFFFF;                      /* Ensure CRC remains 16-bit value */
    }                                        /* Loop until wDataLen=0 */

    return (wCRC);                           /* Return updated CRC */
}


------
//测试数据
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

extern int  TCPServerInit(uint16_t port,uint16_t backlog);

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;


int MyRecv( int iSock, char * pchBuf, size_t tCount){

        size_t tBytesRead=0;
        int iThisRead;
        while(tBytesRead < tCount){
              do{
                     iThisRead = read(iSock, pchBuf, tCount-tBytesRead);
              } while((iThisRead<0) && (errno==EINTR));

              if(iThisRead < 0){
                      return(iThisRead);
              }else if (iThisRead == 0)
                      return(tBytesRead);
              tBytesRead += iThisRead;
              pchBuf += iThisRead;

       }

		
}

void decompose(int32_t  buf[], int n)
{
    //printf("32-bit Word: I : Q\n");
	 int16_t i;
	 int16_t q;
    for(int j=0;j<n;j++)
    {
         int32_t val = buf[j];
         char *p_dat=(char *)&val;
      	 i = ((int16_t*)p_dat)[0]; // Real (I)
	  	 q = ((int16_t*)p_dat)[1]; // Imag (Q)
	     //printf("0x%.8X : %8d : %8d\n",val,i,q);
		 printf("%d,%d\n",i,q);
    }
}

void decomposeBuf(int8_t  recvbuf[], int num)
{
	int32_t content_buf[4000];
	int count = 0;	
	for(int i=0;i<num;i+=4)
	{
		content_buf[count]=*(int32_t*)(&recvbuf[i]);
		//printf("0x%.8X \n",content_buf[count]);
		count++;
	
	}
	//printf("count=%d\n",count);	
	decompose(content_buf,count);
}

int main()
{
	int32_t listenfd=TCPServerInit(8000,5);//端口为8000，连接队列为5
    int32_t connectfd;     				   //socket描述符

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
    int8_t recvbuf[8000];
	
	
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
		   //for(int i=0;i<ret;i++)
		   //{
		   //  printf("%x  ",*(u8*)(&recv_buf[i]));
		   //} 
		   //printf("\r\n");
		   if(*(u16*)(&recv_buf[0])==0x7e7e)
		   {
			   
			   u32 packet_len = *(u32*)(&recv_buf[2]); //content len + tail len
			   data = malloc(packet_len);  //开辟实际长度的data
			   //printf("len = %d \r\n",packet_len);
			   //ret=recv(connectfd, data,packet_len, 0);// 接收数
			   ret=MyRecv(connectfd, data,packet_len);
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
					decomposeBuf(data,ret-2);//去除0D0A
				    //printf("content+tail len%d,%0X  %0X\r\n",ret,*(u8*)(&data[ret-2]),*(u8*)(&data[ret-1]));
			  }
		  }
		}
/*
		num = ret;
		if(num > 0)
		{
			printf("num=%d\n",num);
			//decompose(recvbuf,num);
			for(int i=0;i<num;i+=4)
			{
				//printf("0x%.8X \n",recvbuf[i]);				
				
			}
			
		}
*/		
 	}
	return 0;
}

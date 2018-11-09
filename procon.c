//生产者将生产的数据封装成1个结构体，放入队列中
/*消费者通过从队列中取出1个结构体，之后将数据进行reinterpret_cast解析处理(记得释放内存)
  当消费者无法取到数据时，利用信号量睡眠，能够显著降低CPU利用率
  同时，由于生产者和消费者线程顺序是不一定的，因此利用信号量能够达到同步关系
*/
两个不同的作用域空间，但是利用单例模式获得的是同一个指向结构体对象的对象指针，从而达到全局临界区的访问
后续更改：放入的数据根据项目需要进行修改，尽量优雅的处理
CPU利用率查看：top
内存泄漏检测：/home/pp/software/valgrind/bin/valgrind --track-fds=yes --leak-check=full ./main


----
//CMsgQueue.cpp
/*
 * CMsgQueue.cpp
 *
 *  Created on: Sep 18, 2017
 *      Author: wanglin
 */
#include "CMsgQueue.h"
CMsgQueue* CMsgQueue::_instance;

CMsgQueue* CMsgQueue::get_instance()
{
    if (NULL == _instance) {
        _instance = new CMsgQueue;
    }

    return _instance;
}
void CMsgQueue::push(CMDREQ req)
{
    while(1){
        if (mutex_lock.try_lock()) {
            cmd_queue.push_back(req);
            mutex_lock.unlock();
            break;
        }
    }
}
int CMsgQueue::pop(CMDREQ *req)
{
    if (mutex_lock.try_lock()) {
        *req = cmd_queue.front();
        cmd_queue.pop_front();
        mutex_lock.unlock();
        return 0;
    }else{
        return -1 ;
    }
}
//static_cast() reinterpret_cast()
void CMsgQueue::empty()
{
    CMDREQ_QUEUE::iterator plist;
    if (mutex_lock.try_lock()) {
        for(plist = cmd_queue.begin(); plist != cmd_queue.end(); plist++) {
            //delete plist;
            operator delete(plist->buf);
        }
        cmd_queue.clear();
        mutex_lock.unlock();
    }

}

uint32_t CMsgQueue::size()
{
    return cmd_queue.size();
}


----
//CMsgQueue.h
#ifndef CMSGQUEUE_H_
#define CMSGQUEUE_H_
#include <stdint.h>
#include <stdio.h>
#include <list>
#include <mutex>

using namespace std;

typedef struct  _CMDREQ{
    //int32_t connfd;
    uint32_t index;
    //uint32_t len;
    //uint8_t protocol;
    //CTransmit *dev;
    void* buf;
}CMDREQ;

typedef list<CMDREQ> CMDREQ_QUEUE;

class CMsgQueue {
private:
    CMsgQueue(){};
public:
    ~CMsgQueue(){};
public:
    static CMsgQueue* get_instance();
    void push(CMDREQ req);
    int pop(CMDREQ *req);
    void empty();
    uint32_t size();
private:
    mutex mutex_lock;
    static CMsgQueue* _instance;
    CMDREQ_QUEUE cmd_queue;
};

#endif /* CMSGQUEUE_H_ */


----
//main.cpp
#include "CMsgQueue.h"
#include <arpa/inet.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

sem_t cmd_sem;

//生产者，只要环形队列有空位,便不断生产
void*productor(void*arg){
	int data = 0;

	CMsgQueue *pqueue;
	pqueue = CMsgQueue::get_instance();

	
  	CMDREQ cmd;
	//数据包内容
	uint32_t proIndex = 0;
	size_t num = 1024;
	uint8_t probuf[num] = "helloworld";
	while(1) {
		//
		proIndex++;
		if(100000 == 0)
			proIndex = 0;

		//
		cmd.buf = operator new(sizeof(uint8_t) * num);
		if(cmd.buf == NULL){
			printf("recive thread buffer alloc failed\n");
			return NULL;
		}
		cmd.index = proIndex;
		memcpy(cmd.buf, probuf,num);

		//
		pqueue->push(cmd);
		sem_post(&cmd_sem);

		usleep(1);
	}
	printf("protductor\n");
}

//消费者,只要环形队列中有数据,就不断消费
void*consumer(void*arg){
	size_t num = 1024;
  	uint8_t *conbuf = NULL;
    int conIndex = 0;

	CMsgQueue *pqueue;
	pqueue = CMsgQueue::get_instance();
    
	CMDREQ cmd;
	while(1) {
        if(pqueue->size()>0) {
			//printf("size:%d\n",pqueue->size());

            // Get one command.
            if(pqueue->pop(&cmd)<0){
                continue;
            }
			conbuf = reinterpret_cast<uint8_t*>(cmd.buf);
			conIndex = cmd.index;
			printf("index:%d buf:%s\n",conIndex,conbuf);

			operator delete(cmd.buf);
		}
		else {
			//实际情况时消费者一直等待直到临界区有数据（等待5s)
						
			struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                printf("clock_gettime failed\n");
                //sem_wait(&cmd_sem);
                usleep(1000);
            }else{
                ts.tv_sec += 5;
                sem_timedwait(&cmd_sem, &ts);
            }
			/**/
		}
	}
	
	printf("consumer\n");

}
int main() 
{
	sem_init(&cmd_sem,0,0);

	pthread_t pro,con;
	pthread_create(&pro,NULL,productor,NULL);
   	 pthread_create(&con,NULL,consumer,NULL);
	pthread_join(pro,NULL);
    	pthread_join(con,NULL);
 
  
	return 0;
}

----
命令
g++ main.cpp CMsgQueue.cpp -std=c++11 -o main -lpthread

//此代码有个缺点：若生产者生产速率特别快，将内存占用完了死机了怎么办？

#include "CMsgQueue.h"
#include <arpa/inet.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <thread>

sem_t cmd_sem;

class Productor {

public:
	Productor() {
		pqueue = NULL;
	}
	~Productor() {
		delete pqueue;
		pqueue = NULL;

		delete pdatatid;
		pdatatid = NULL;
	}	
	void init();
	void productData();
private:
	CMsgQueue *pqueue;
	thread *pdatatid;
};


void Productor::productData() {

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
			return;
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

static void createProthread(Productor *p) {
	//调用成员函数
	p->productData();
}
void Productor::init() {

	pqueue = CMsgQueue::get_instance();

	pdatatid = new thread(createProthread,this);
	pdatatid->detach();
}


class Consumer {

public:
 	Consumer() {
		pqueue = NULL;
	}
	~Consumer() {
		delete pqueue;
		pqueue = NULL;
	}
	void execute(void);
private:
    CMsgQueue *pqueue;
};

void Consumer::execute(void) {
	size_t num = 1024;
  	uint8_t *conbuf = NULL;
    int conIndex = 0;

	CMsgQueue *pqueue;
	pqueue = CMsgQueue::get_instance();
    
	CMDREQ cmd;
	while(1) {
        if(pqueue->size()>0) {
			//printf("size:%d  ",pqueue->size());

            // Get one command.
            if(pqueue->pop(&cmd)<0){
                continue;
            }
			conbuf = reinterpret_cast<uint8_t*>(cmd.buf);
			conIndex = cmd.index;
			printf("size:%d index:%d buf:%s\n",pqueue->size(),conIndex,conbuf);

			operator delete(cmd.buf);
			//usleep(1);
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
		}
	}
	
	printf("consumer\n");

}



//生产者，只要环形队列有空位,便不断生产
void*productor(void*arg){

	pthread_detach(pthread_self());
	
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

		//usleep(1);
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
			printf("size:%d  ",pqueue->size());

            // Get one command.
            if(pqueue->pop(&cmd)<0){
                continue;
            }
			conbuf = reinterpret_cast<uint8_t*>(cmd.buf);
			conIndex = cmd.index;
			printf("index:%d buf:%s\n",conIndex,conbuf);

			operator delete(cmd.buf);
			usleep(1);
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
	Productor *pt = NULL;
	if(NULL == pt) {
		pt = new Productor();
	}
	
	if(NULL != pt) {
		//生产者类 生产数据
		pt->init();
		//消费者线程消费数据(可扩展为消费者类,在主线程执行)
		Consumer con;
		con.execute();
		//二者通过数据类和信号量同步
	}
//	pthread_t pro,con;
//	pthread_create(&pro,NULL,productor,NULL);
//  pthread_create(&con,NULL,consumer,NULL);
//	pthread_join(pro,NULL);
//  pthread_join(con,NULL);
 
  
	return 0;
}

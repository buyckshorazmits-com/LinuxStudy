#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>
#include<time.h>
#include<pthread.h>
//计数信号量主要是用于控制进程或线程执行顺序的
sem_t *empty;
sem_t *full;
int share_num;
 /*
 share_num作为生产者和消费者之间共享的缓冲区。
 empty:
        表示缓冲区剩余空位置数量。
        初始值为1，表示可以生产一个数据。

    full:
        表示缓冲区已有数据数量。
        初始值为0，表示当前没有数据可消费。*/

int rand_num()
{
  srand(time(NULL));

  return rand();
}
void *producer(void *argv)
{
  for(int i=0;i<5;i++)
  {
    sem_wait(empty);
    printf("第%d轮数据传输\n",i+1);//
    //① 生产者的逻辑：首先等待缓冲区为空，然后写入数据，最后通过sem_post告诉消费者，缓冲区已满，可以消费数据。//
    sleep(1);
    share_num=rand_num();
    printf("producer has sent data\n");
    sem_post(full);
  }
}
void *consumer(void *argv)
{
  for(int i=0;i<5;i++)
  {
   //② 消费者的逻辑：首先等待缓冲区满，然后消费数据，最后通过sem_post告诉生产者，缓冲区已空，可以生产数据
    sem_wait(full);
  printf("consumer has read data\n");
  printf("the share_num is  %d ",share_num);

  sleep(1);
  sem_post(empty);
  }
  
}
int main()
{
  //初始时缓冲区是空的，因此，empty信号量的初值应为1，而full应为0，
  //二者都是用于线程间通信的，sem_init()函数的第二个参数都应该是0。
  full=malloc(sizeof(sem_t));
  empty=malloc(sizeof(sem_t));

  sem_init(empty,0,1);//empty=1,信号量初始许可（资源数量）的个数。
  sem_init(full,0,0);//full=0,信号量初始许可（资源数量）的个数。

  pthread_t producer_id,consumer_id;

  pthread_create(&producer_id,NULL,producer,NULL);
  pthread_create(&consumer_id,NULL,consumer,NULL);

  
  pthread_join(producer_id,NULL);
  pthread_join(consumer_id,NULL);

  sem_destroy(empty);
  sem_destroy(full);

  return 0;
}
/*代码基本逻辑
生产者通过 sem_wait(empty) 消耗一个空位置许可，将 empty 从1减为0，然后生产数据。
由于此时 full 仍然为0，消费者线程在 sem_wait(full) 处阻塞，无法读取缓冲区。只有生产者完成数据写入，
并通过 sem_post(full) 将 full 从0增加到1后，消费者才获得读取许可，继续执行消费操作*/
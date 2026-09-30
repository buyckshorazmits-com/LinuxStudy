#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define THREAD_COUNT 20000
//避免竟态条件
//（1）避免多线程写入一个地址。
//（2）给资源加锁 像互斥锁，使同一时间操作特定资源的线程只有一个。

void *add_thread(void *argv)
{
  int *p =argv;
  (*p)++;//所有线程传的都是同一个地址
  return (void *) 0;
}

int main()
{
  pthread_t pid[THREAD_COUNT];

  int num=0;
  //用20000个线程对num累加
//同一地址 + 共享写操作 + 并发执行 + 无同步保护 = 可能产生竞态条件。
//不同线程对于num的累加操作可能重叠，这就会导致多次累加操作可能只生效一次。导致最终的自加次数会小于20000
  for(int i=0;i<THREAD_COUNT;i++)
  {
    //很多线程通过这个同一个地址，去并发修改同一个共享变量，而且没有同步保护。
    pthread_create(pid+i,NULL,add_thread,&num);
 
  }
  //等待所有线程结束
  for(int i=0;i<THREAD_COUNT;i++)
  {
    //pthread_join() 是在所有线程都创建完以后才执行
    pthread_join(pid[i],NULL);

  }
  //打印累加结果
  printf("累加结果:%d \n",num);

  return 0;
}
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

#define PTHREAD_COUNT 20000

static pthread_mutex_t  counter_mutex= PTHREAD_MUTEX_INITIALIZER;

void *add_pthread(void *argv)
{
  int *p=argv;
  //累加之前加锁，其他线程访问就会阻塞
  pthread_mutex_lock(&counter_mutex);
  (*p)++;
  //累加之后释放锁
  pthread_mutex_unlock(&counter_mutex);

  return (void*) 0;
}
int main()
{
  pthread_t tid[PTHREAD_COUNT];

  int num=0;
  for(int i=0;i<20000;i++)
  {
    pthread_create(tid+i,NULL,add_pthread,&num);
  }

  //等待20000个线程结束  回收资源
  for(int i=0;i<20000;i++)
  {
    pthread_join(tid[i],NULL);
  }
  printf("累加结果：%d \n",num);

  return 0;
}
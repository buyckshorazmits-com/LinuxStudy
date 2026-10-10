#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

int share_num=0;

void *plusOne(void *argv)
{
  int tmp=share_num+1;
  share_num=tmp;

}
int main()
{
  pthread_t tid[1000];
  for(int i=0;i<1000;i++)
  {
   pthread_create(tid+i,NULL,plusOne,NULL);

  }
  for(int i=0;i<1000;i++)
  {
    pthread_join(tid[i],NULL);
  }

  printf("share_num is %d\n",share_num);

  return 0;

}
//多次执行，我们会发现，所有线程执行完毕之后，shared_num的值并不总是10000，这是因为shared_num的加一并不是原子操作，
//如果某个线程执行了int num=shared_num+1，在它将tmp的值赋给shared_num之前，别的线程执行了第一步，在这里产生了竞争
//产生竟态条件
//那么加一操作就会被“吞掉”，最终导致10000次加一的结果并未全部生效，因而shared_num小于10000。
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
void *task(void *arg)
{
  printf("Thread started\n");
  sleep(2);//模拟线程工作
  printf("Thread finised\n");

  return NULL;
}
int main()
{
 pthread_t tid ;
 //创建线程
 pthread_create(&tid,NULL,task,NULL);

 //使用pthread_detach让线程自动回收资源  
 pthread_detach(tid);

 //调用时（t≈0）什么资源都不回收，它只是把子线程标记为 detached；
 //真正的回收发生在子线程 task() 执行到 return NULL 的那一瞬间（实测 t≈2.0），由内核 + glibc 在线程退出路径上自动完成。

 //主线程继续工作
 printf("Main pthread continues\n");

 sleep(3);
// 需要注意的是，pthread_detach不会等待子线程结束，如果在后者执行完毕之前主线程退出，则整个进程退出，
//子线程被强制终止，因此需要等待足够的时间确保子线程完成自己的任务

  printf("Main pthread end\n");

}
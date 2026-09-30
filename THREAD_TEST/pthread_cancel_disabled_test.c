#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

void *task(void *arg)
{
  //可能是取消点：printf()、fprintf()、fopen()，fclose()等标准I/O 函数。
  //具体实现可以把它们作为取消点
  printf("thread started!\n");

  //禁用取消响应
  //子线程不接受取消  相当于一个总开关
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  //请求保持 pending
  printf("thread Cancel is disabled\n");

  //设置取消类型为异步
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
  
  //即使主线程执行：

//pthread_cancel(tid);
//这个取消请求也不会让它现在终止  在这段代码中  会直接执行完
  //模拟工作
  printf("thread working\n");
  sleep(1);

  printf("After canceled\n");

  return NULL;
}
int main()
{
pthread_t tid;
void *res;
//创建线程
pthread_create(&tid,NULL,task,NULL);
//取消线程
if(pthread_cancel(tid)!=0)
{
  perror("pthread_cancel");
}
//等待子线程终止并获取结束状态
pthread_join(tid,&res);
//检查一下子线程是否被取消

if(res==PTHREAD_CANCELED)
{
  printf("thread was canceled\n");
}
else
{
  printf("thread was not canceled,exit code:%ld \n",(long)res);
}
return 0;


}
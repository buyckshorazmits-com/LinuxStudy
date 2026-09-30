#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

void *task(void *arg)
{
printf("thread started\n");
//设置取消类型为异步
pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
//异步取消：只要收到取消请求，就可以立即终止，不需要等到某个取消点
printf("thread working \n");
//创建线程之后，主线程和子线程是并发执行的，谁先执行到哪一步是不确定的。
sleep(1);

printf("After canceled\n");

return NULL;

}
int main()
{
pthread_t tid;

void *res;
//创建子线程
pthread_create(&tid,NULL,task,NULL);
//取消子线程
if(pthread_cancel(tid)!=0)
{
  perror("pthread_cancel");
}

//等待子线程终止  并获取推出状态
pthread_join(tid,&res);

if(res==PTHREAD_CANCELED)
{
printf("pthead was canceled\n");
}
else{
  printf("pthread was not canceled ,exit code:%ld",(long)res);
}
return 0;
}
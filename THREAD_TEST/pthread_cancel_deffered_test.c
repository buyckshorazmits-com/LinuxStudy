/*
程序运行逻辑：

1. main()函数首先创建一个子线程(task线程)：
   
   pthread_create(&tid,NULL,task,NULL);

   创建成功后，程序中同时存在：
   - 主线程(main线程)
   - 子线程(task线程)


2. task线程开始执行task()函数：

   （1）打印线程启动信息；
   
   （2）模拟线程执行任务：
       printf("Working .....");
       sleep(1);

       sleep()属于线程取消点，线程在这里会检查是否收到取消请求。


   （3）执行pthread_testcancel();

       该函数用于主动检测线程是否收到取消请求。
       如果main线程之前调用pthread_cancel()发送了取消请求，
       那么task线程会在这里被终止，后续代码不会继续执行。


3. main线程创建task线程后，会调用：

   pthread_cancel(tid);

   向task线程发送取消请求。

   注意：
   pthread_cancel()不会立即杀死线程，
   它只是发送一个取消信号。
   在线程运行到取消点(sleep、pthread_testcancel等)时，
   线程才会真正退出。


4. main线程调用：

   pthread_join(tid,&res);

   等待task线程结束，并获取线程退出状态。

   如果task线程：
   
   （1）正常执行结束：
       return NULL;
       则res保存线程返回值(NULL)。

   （2）被pthread_cancel()取消：
       则res保存PTHREAD_CANCELED。


5. main线程通过判断：

   if(res==PTHREAD_CANCELED)

   判断task线程是否是因为取消请求而退出。

   如果相等：
       说明线程被pthread_cancel()成功取消。

   否则：
       说明线程正常结束。


整体流程：

main线程
    |
    | pthread_create()
    ↓
创建task线程
    |
    ↓
task线程执行任务
    |
    |
main线程调用pthread_cancel()
    |
    ↓
发送取消请求
    |
    ↓
task线程到达取消点
    |
    ↓
检测到取消请求并退出
    |
    ↓
pthread_join()等待并获取退出状态
    |
    ↓
判断res，确认线程退出原因

核心知识点：
pthread_cancel() = 发送取消请求
pthread_testcancel() = 检查取消请求
pthread_join() = 等待线程结束并获取退出结果
PTHREAD_CANCELED = 线程被取消后的返回状态
*/

#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

void *task(void *arg)
{
  printf("Thread started\n");
  //默认的取消类型为延迟  不用设置

  //模拟工作
  printf("Working .....\n");
  sleep(1);//模拟工作

  pthread_testcancel();//取消点函数

  printf("After canceled\n");
  //After Cancelled并未被打印，且主线程执行了取消成功分支，子线程被成功取消。
   
  return NULL;
}
int main()
{
   pthread_t tid;

   void *res;

   //创建线程
   pthread_create(&tid,NULL,task,NULL);

   //取消子线程
   if(pthread_cancel(tid)!=0)
   {
    perror("pthread_cancel");
   }
   //等待子线程终止并获取状态
   pthread_join(tid,&res);
   //等待子线程结束；
   //把子线程结束时留下的信息保存到 res。

   //检查子线程是否被取消
   if(res==PTHREAD_CANCELED)
   {
    printf("thread was canceled\n");//检查子线程是否是 因为取消请求而取消成功
   }
   else
   {
    printf("thread was not canceled ,exit code:%ld",(long)res);  

   }
    return 0;
}
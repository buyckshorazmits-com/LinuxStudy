#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>

//定义信号处理函数
void sigint_handler(int signum)
{
  printf("\n 收到%d信号,停止程序\n",signum);
  exit(signum);//不管程序当前执行到哪里，直接结束整个程序。
}
/**
 *  signal系统调用会注册某一信号对应的处理函数。如果注册成功，当进程收到这一信号时，将不会调用默认的处理函数，而是调用这里的自定义函数
 * 
 * int signum: 要处理的信号
 * sighandler_t handler: 当收到对应的signum信号时，要调用的函数
 * return: sighandler_t 返回之前的信号处理函数，如果错误会返回SEG_ERR
 */

int main()
{
  //注册SIGNUM信号处理函数 收到ctrl+c信号之后不再执行默认的函数，而是执行新的注册函数
  if(signal(SIGINT,sigint_handler)==SIG_ERR)
  {
    //（1）SIGINT（2）：这是当用户在终端按下Ctrl+C时发送给前台进程的信号，通常用于请求进程终止。
    printf("注册新的信号处理函数失败\n");
    return 1;

  }
  while(1)
  {
    sleep(1);
    printf("你好 在吗？\n");
  }
  return 0;
}
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>

int main(int argc, char const *argv[])
{
  if(argc<2)
  {
    //argc 是新程序启动后，系统根据你传进去的 args[] 自动计算出来的。
    printf("参数不够，不能上二楼。\n");
    return 1;
  }
  printf("我是%s %d,我跟海哥上二楼了\n",argv[1],getpid());
  //挂起子进程
  sleep(100);//父进程结束后 子进程还在挂起  制造了孤儿进程
  return 0;
}

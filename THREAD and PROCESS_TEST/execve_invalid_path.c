#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main()
{
  char *argv[]={"ping","-c","10","baidu.com",NULL};

  //环境变量不可传
  char *envp[]={NULL};

  int re=execve(argv[0],argv,envp);
  
  if(re==-1)
  {
    perror("execve");
    return -1;
  }

  return 0;
  //未在execve 中指明ping的路径  编译运行报错
  
}
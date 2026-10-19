#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<unistd.h>

int main()
{
  pid_t son_pid=fork();
   
  setpgid(getpid(),getpid());
  // 通过setpgid()将PID设置为各自的PGID，两个进程各自创建一个进程组。
  //两个进程各自创建了一个进程组，进程组ID等于各自的进程ID。
  //父进程和子进程 各创建一个进程
  if(son_pid>0)
  {
    waitpid(son_pid,NULL,0);
  }
  else if(son_pid==0)
  {
    char *str=malloc(100);

    fgets(str,100,stdin);

    printf("子进程收到数据:%s",str);

    free(str);

    str=NULL;
  }
  else
  {
    perror("fork");
  }

  return 0;
}
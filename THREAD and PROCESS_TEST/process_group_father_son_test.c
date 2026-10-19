#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>

int main()
{

  pid_t son_pid=fork();

  //fork 之后子进程继承父进程的 PGID 和 SID，两个进程天然处于同一个进程组中；
  
  if(son_pid>0)
  {
    waitpid(son_pid,NULL,0);
  }
  else if(son_pid==0)
  {
    char *str=malloc(100);

    fgets(str,100,stdin);

    printf("收到子进程数据: %s \n",str);
    
    free(str);

    str=NULL;
  }
  else
  {
    perror("son_pid");
  }

  return 0;
}
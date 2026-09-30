#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
int main()
{
  printf("海哥教老学员%d春暖花开\n",getpid());
  pid_t pid=fork();
  if(pid<0)
  {//创建新进程失败
    printf("新学员加入失败\n");
    return 1;
  }else if(pid==0)
  {
    //进入子进程
    printf("新学员%d加入成功,是老学员%d推荐\n",getpid(),getppid());
  }
  else
  {
    printf("老学员%d继续深造,他推荐了%d\n",getppid(),pid);
  }

  return 0;
}
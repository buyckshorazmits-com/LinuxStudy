#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char const *argv[])
{
  char *name="老学员";
  printf("%s %d在一楼精进\n",name,getpid());
  __pid_t pid =fork();
  if(pid==-1)
  {
    printf("邀请新学员失败\n");
  }else if(pid==0)
  {
    char *newName="xiaowang";
    char *args[]={"/home/linux/vscoding/FLIE_IO/erlou",newName,NULL};
    char *envs[]={"/home/linux/work/tools/gcc_riscv32/bin:/home/linux/.local/bin:/home/linux/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin",NULL};
    int re=execve(args[0],args,envs);
    if(re==-1)
    {
      printf("新学员上二楼失败\n");
      return 1;
    }
  }else
  {
    //老学员在这里
    printf("老学员%d 邀请完%d 之后还是在一楼学习\n",getpid(),pid);
  }
  return 0;
}

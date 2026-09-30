#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>

int main(int argc, char const *argv[])
{
  //fork()之前
  int subprocess_status;
  printf("老学员在校区\n");
  pid_t pid = fork();
  if(pid<0)
  {
    perror("fork");
    return 1;
  }
  else if(pid==0)
  {
    //新学员
    char *args[]={"/usr/bin/ping","-c","50","www.linux.com",NULL};
    char *envs[]={NULL};
    printf("新学员%d联系海哥10次\n",getpid());
    int exr=execve(args[0],args,envs);
    if(exr<0)
    {
      perror("execve");
      return 1;
    }
  }
  else
  {
    printf("老学员%d 等待新学员%d 联系\n",getpid(),pid);
    waitpid(pid,&subprocess_status,0);
  }
  printf("老学员等待新学员联系完成\n");
  return 0;
}

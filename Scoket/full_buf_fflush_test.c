#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>

int main()
{
  FILE *file =fopen("/home/linux/vscoding/Scoket/testfile.txt","w");

  if(file==NULL)
  {
    perror("Failed to open file");
    return 1;
  }

  //设置文件为全缓冲模式
  if(setvbuf(file,NULL,_IOFBF,0)!=0)//全缓冲模式 _IOFBF
  {
    perror("Failed to set buffer mode");
    return 1;
  }

  fprintf(file,"hello\n");

  fflush(file);
  //添加了fflush()，即便设置为全缓冲模式，hello\n也会被刷写入目标文件，此时testfile.txt中可以看到hello和换行。

  char *argv[]={"/usr/bin/ping","-c","1","www.baidu.com",NULL};

  char *envp[]={NULL};

  execve("/usr/bin/ping",argv,envp);

  perror("execve");

  return 0;
}
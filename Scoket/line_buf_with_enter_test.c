#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>
//testfile.txt会写入hello和换行符。
int main()
{
  FILE *file=fopen("/home/linux/vscoding/Scoket/testfile.txt","w");

  if(file==NULL)
  {
    perror("Failed to open file");

    return 1;
  }
  //设置文件为行缓冲模式
  if(setvbuf(file,NULL,_IOLBF,0)!=0)
  {
    perror("Failed to set buffer mode");

    return 1;
  }
  fprintf(file,"hello\n");

  char *argv[]={"/usr/bin/ping","-c","1","www.baidu.com",NULL};
  char *envp[]={NULL};
  
  execve("/usr/bin/ping",argv,envp);
  perror("execve");

  return 0;

}
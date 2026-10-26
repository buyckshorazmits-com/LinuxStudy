#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>

int main()
{
  FILE *file=fopen("/home/linux/vscoding/Scoket/testfile.txt","w");
//清除文本文件testfile.txt 中以前的内容
  if(file==NULL)
  {
    perror("Failed to open file");
    return 1;
  }

  //设置文件为无缓冲模式
  if(setvbuf(file,NULL,_IONBF,0)!=0)
  {
    perror("Failed to set buffer mode");
    return 1;

  }

  fprintf(file,"hello");//让 fprintf() 等标准 I/O 操作不再通过通常的 stdio 输出缓冲  不等待stdio缓冲区填满 通过底层写操作将字符串交给内核。

  char *argv[]={"/usr/bin/ping","-c","1","www.baidu.com",NULL};

  char *envp[]={NULL};

  execve("/usr/bin/ping",argv,envp);
  //当前进程的pid不变 替换的是正在运行的程序 旧代码段、堆、栈、数据段等被新程序映像取代  之前修改的文件还是原来一样
  perror("execve");

  return 0;
}
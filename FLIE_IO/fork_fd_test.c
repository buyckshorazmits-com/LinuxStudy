#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
int main(int argc, char const *argv[])
{
  //进入fork()之前打开一个文件
  int fd=open("io.text",O_CREAT | O_WRONLY | O_APPEND,0644);
  if(fd==-1)
  {
    perror("open");
    exit(EXIT_FAILURE);
  }
  char buffer[1024];
  pid_t pid=fork();

if(pid<0)
{
  perror("fork");
  exit(EXIT_FAILURE);
}
else if(pid==0)
{
  // 子进程代码
  strcpy(buffer,"这是子进程要写入的数据！\n");
}
else
{
  //父进程代码
  sleep(1);
  strcpy(buffer,"这是父进程写入的数据\n");
}
// 父子进程都要执行的代码
ssize_t bytes_write=write(fd,buffer,strlen(buffer));
if(bytes_write==-1)
{
  perror("write");
  close(fd);
  exit(EXIT_FAILURE);
}
printf("数据写入成功\n");
// 使用完毕之后关闭
close(fd);
if(pid==0)
{
  printf("子进程写入完毕，释放文件描述符\n");
}else
{
  printf("父进程写入完毕，并释放文件描述符\n");
}
  return 0;
}


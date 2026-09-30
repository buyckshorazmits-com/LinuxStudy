#include <fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char const *argv[])
{
  int fd;
  char *pipe_path="/tmp/myfifo";
  //打开有名管道进行读取
  fd=open(pipe_path,O_RDONLY);
  if(fd==-1)
  {
    perror("open failed");
    exit(EXIT_FAILURE);
  }
  char read_buf[100]={0};
  ssize_t read_num;
  while((read_num=read(fd,read_buf,100))>0)
  {
    write(STDOUT_FILENO,read_buf,read_num);
  }
  if(read_num<0)
  {
    perror("read failed");
    printf("命令行读取异常\n");
    close(fd);
    exit(EXIT_FAILURE);
  }
  printf("接受管道退出进程终止\n");
  close(fd);
  
  return 0;
}

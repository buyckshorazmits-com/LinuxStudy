#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include <sys/stat.h>
#include<errno.h>
int main(int argc, char const *argv[])
{
  int fd ;
  char *pipe_path="/tmp/myfifo";
  //创建有名管道
  if(mkfifo(pipe_path,0644) !=0 )
  {
    if(errno!=EEXIST)//errno==EEXIST表示文件已存在
    {
      perror("mkfifo failed");
      exit(EXIT_FAILURE);
    }
  }
  //打开有名管道  写入
  fd =open(pipe_path,O_WRONLY);
   if(fd==-1)
   {
    perror("open failed");
    exit(EXIT_FAILURE);
   }

   char write_buf[100]={0};
   ssize_t read_num;
   while((read_num=read(STDIN_FILENO,write_buf,100))>0)
   {
    write(fd,write_buf,read_num);
   }
   if(read_num<0)
   {
    perror("read  failed");
    printf("命令行读取数据异常\n");
    close(fd);
    exit(EXIT_FAILURE);
   }
   printf("发送管道退出  进程终止\n");
   close(fd);
   if(unlink(pipe_path)==-1)
   {
    perror("fifo_write unlink");
   }
   
  return 0;
}

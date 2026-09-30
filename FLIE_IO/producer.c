#include<time.h>
#include<mqueue.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

int main()
{
  
  char *mq_name ="/p_c_mq";

  struct mq_attr attr;
  attr.mq_flags=0;
  attr.mq_maxmsg=10;
  attr.mq_msgsize=100;
  attr.mq_curmsgs=0;
  
  //创建或打开消息队列 
  mqd_t mqdes =mq_open(mq_name,O_WRONLY | O_CREAT,0666,&attr);

  if(mqdes==(mqd_t)-1)//mqd_t 成功则返回消息队列描述符，失败则返回(mqd_t)-1，同时设置errno以指明错误原因
  {
    perror("mq_open");
    exit(EXIT_FAILURE);
  }
  char writeBuf[100];
  struct timespec time_info;

  while(1)
  {
    //清空写缓冲区
    memset(writeBuf,0,100);

    //从命令行标准输入读取数据
    ssize_t read_count=read(0,writeBuf,100);//这里的0代表STDIN_FILENO 表示键盘输入
    if(read_count==-1)
    {
      perror("read");
      continue;
    }
    else if(read_count==0)
    {

    }
    //获取当前时间中5S后的timespec对象

   clock_gettime(CLOCK_REALTIME,&time_info);
    time_info.tv_sec+=5;
    
    //发送数据
    //如果接受到命令行的EOF，read将返回0,此时向消费者端发送消息并退出
    if(read_count==0)
    {
      printf("Receive EOF,exit...\n");
      char eof=EOF;
      //当程序检测到输入结束时，向消息队列发送一个“EOF结束标志”，然后退出当前循环。
      if(mq_timedsend(mqdes,&eof,1,0,&time_info)== -1)
      {
       perror("mq_timedsend");
      }
      break;
    }
     //若没有接收到EOF ，正常发送数据
     if(mq_timedsend(mqdes,writeBuf,strlen(writeBuf),0,&time_info)== -1)
     { 
      perror("mq_timedsend");
     }
     printf("从命令行接受到数据 ，发送到消费者端\n");
  }
  //关闭描述符
  mq_close(mqdes);
  //mq_unlink 只应调用一次  选择在消费者端完成
  return 0;
}
#include<fcntl.h>
#include<sys/stat.h>
#include<mqueue.h>
#include<stdio.h>
#include<time.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
int main(int argc, char const *argv[])
{
  
  //创建消息队列
  struct mq_attr attr;
  //消息队列的容量 
  attr.mq_maxmsg =10 ;
  attr.mq_msgsize =100;

  attr.mq_flags=0;//消息队列的标志位 表示消息队列描述符是否采用非阻塞模式  mq_flags=0表示阻塞模式
  attr.mq_curmsgs=0;

  char *mq_name="/father_son_mq";
  mqd_t mqdes =mq_open(mq_name,O_RDWR | O_CREAT ,0644,&attr);

  if(mqdes==(mqd_t)-1)
  {
    perror("mq_open");
    exit(EXIT_FAILURE);
  }
  //创建父子进程
  pid_t pid=fork();
  if(pid<0)
  {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if(pid==0)
  {
    //接受消息队列中的信息
    char read_buf[100];
    struct timespec time_info;
    for(size_t i=0;i<10;i++)
    {
      //清空接受数据的缓冲区
      memset(read_buf,0,100);
      //获取当前时间
      clock_gettime(0,&time_info);
      //设置接受数据的等待时间  在当前时间的基础上加五秒，表示最晚等到当前时间的五秒后就不再等待
      time_info.tv_sec+=5;
      //接收到消息队列的数据  打印到控制台
      if(mq_timedreceive(mqdes,read_buf,100,NULL,&time_info)==-1)//接受成功返回接收到的字节数，失败返回-1
      {
        perror("mq_timedreceive");

      }
      printf(" 子进程接收到数据：%s\n",read_buf);
    }
    
  }
  else
  {
    // 父进程  发送消息到消息队列中
    char send_buf[100];
    struct timespec time_info;
    for(size_t i=0;i<10;i++)
    {
      //清空缓冲区的数据
      memset(send_buf,0,100);
      sprintf(send_buf,"父进程的第%d次发送消息\n",(int)(i+1));
      //获取当前具体时间
      clock_gettime(0,&time_info);
      time_info.tv_sec+=5;
      //发送消息
      if(mq_timedsend(mqdes,send_buf,strlen(send_buf),0,&time_info)==-1)
      {
        perror("mq_timedsend");
      }
      printf("父进程发送一条消息，休眠1s\n");
      sleep(1);
      
    }
  }
  // 最终不管是父进程还是子进程都需要释放消息队列的引用
  mq_close(mqdes);
  if(pid > 0)
  {
    mq_unlink(mq_name);
  }
  return 0;
  
}

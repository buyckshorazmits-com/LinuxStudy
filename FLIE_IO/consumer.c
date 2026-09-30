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
  mqd_t mqdes =mq_open(mq_name,O_RDONLY ,0666,&attr);

  if(mqdes==-1)
  {
    perror("mq_open");
  }
  char readBuf[100];
  struct timespec time_info;

  while(1)
  {
    memset(readBuf,0,100);
    //获取1天后的time_spec 结构对象，目的是在测试期间是的消费者一直等待生产者发送的数据
    clock_gettime(CLOCK_REALTIME,&time_info);
    time_info.tv_sec+=5;

    //接受数据
    if(mq_timedreceive(mqdes,readBuf,100,NULL,&time_info)==-1)
    {
      perror("mq_timedreceive");
      continue;//超时或出错时本轮没有收到任何数据，必须跳过下面的打印，否则会误报"接收到数据"
    }
    //如果收到生产者的发送的EOF，则结束进程
    if(readBuf[0]== EOF)
    { 
      printf("接收到生产者的终止信号，准备退出.. \n");
      break;
    }
    //如果没有收到EOF 将接收到的数据打印到标准输出
    printf("接收到来自生产者的数据\n%s",readBuf);
  }

  //释放消息队列描述符
  mq_close(mqdes);
  //mq_unlink 只应调用一次
  //清除消息队列：注意参数是队列名字符串，不是描述符
  mq_unlink(mq_name);
}
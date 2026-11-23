#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define handle_error(cmd,result) \
    if(result <0)                \
    {                            \
      perror(cmd);               \
      return -1;                 \
    }

void *read_from_server(void *argv)
{
  int sockfd=*(int *)argv;
  char *read_buf=NULL;
  ssize_t count=0;
  read_buf=malloc(sizeof(char )* 1024);

  if(!read_buf)
  {
    perror("malloc client read_buf");
    return NULL;

  }

  while(1)
  {
    count =recv(sockfd,read_buf,1023,0);
    if(count==0)
    {
      break;
    }
    if(count<0)
    {
      perror("recv");
      break;
    }
    read_buf[count]='\0';
    fputs(read_buf,stdout);
  }

  printf("收到服务端的终止信号.....\n");
  free(read_buf);

  return NULL;
}

void *write_to_server(void *argv)
{
  int sockfd=*(int *)argv;
  char *write_buf=NULL;
  ssize_t send_count;

  write_buf=malloc(sizeof(char)* 1024);
  
  if(!write_buf)
  {
    printf("写缓存申请异常，断开连接\n");
    shutdown(sockfd,SHUT_WR);

    perror("malloc client write_buf");

    return NULL;

  }

  // 坑：原来写的是 fgets(write_buf, strlen(write_buf), stdin)
  // 第二个参数是“缓冲区大小”，而 write_buf 是 malloc 的未初始化内存，
  // strlen 通常得 0 → fgets(_,0,_) 直接返回 NULL → 一次输入都没读就退出
  while(fgets(write_buf,1024,stdin)!=NULL)
  {
   // 只发实际读到的字节数，不能固定发 1024（会把缓冲区里的垃圾一起发过去）
   send_count=send(sockfd,write_buf,strlen(write_buf),0);
   if(send_count<0)
   {
   
    perror("send");
   }
  }

  printf("接收到命令行的终止信号，不再写入，关闭连接.....\n");

  shutdown(sockfd,SHUT_WR);

  free(write_buf);

  return NULL;

}


int main(int argc,char const *argv[])
{
  int sockfd,temp_result;

  pthread_t pid_read,pid_write;

  struct sockaddr_in server_addr;
  memset(&server_addr,0,sizeof(server_addr));

  server_addr.sin_family=AF_INET;
  //连接至本机127. 0 .0.1
  server_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  //连接端口6666
  server_addr.sin_port=htons(6666);

  //创建socket
  sockfd=socket(AF_INET,SOCK_STREAM,0);
  handle_error("socket",sockfd);

  //连接server
  temp_result=connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));

  handle_error("connect",temp_result);

  //启动子线程 用于读取服务端的数据，并打印到stdout
  pthread_create(&pid_read,NULL,read_from_server,(void *)&sockfd);
  //启动子线程 用于从命令行里 读取数据发送到服务端
  pthread_create(&pid_write,NULL,write_to_server,(void *)&sockfd);

  //主线程等待子线程关闭
  pthread_join(pid_read,NULL);
  pthread_join(pid_write,NULL);

  printf("关闭资源\n");
  close(sockfd);

  return 0;
}
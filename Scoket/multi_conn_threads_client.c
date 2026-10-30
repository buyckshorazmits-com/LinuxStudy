#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define handle_error(cmd,result)  \
  if(result<0)                    \
  {                               \
      perror(cmd);                \
      return -1;                  \
  }


void *read_from_server(void * argv)
{
  int sockfd =*(int *)argv;
  char *read_buf =NULL;
  ssize_t count =0;

  read_buf= malloc(sizeof(char) *1024);

  if(!read_buf)
  {
    perror("malloc client read_buf");
    return NULL;

  }
  
  //recv 返回值语义: >0 收到的字节数, 0 对端关闭, -1 出错
  //只收 1023 字节, 把最后一个字节留给 '\0'
  while((count=recv(sockfd,read_buf,sizeof(char)*1024-1,0))>0)
  {
    //recv 不会自动补结束符, 而 fputs 依赖 '\0', 必须自己补
    read_buf[count]='\0';
    fputs(read_buf,stdout);
    fflush(stdout);
  }

  if(count<0)
  {
    perror("recv");
  }
  else
  {
    printf("收到服务端的终止信号.....\n");
  }

  free(read_buf);

  return NULL;
}
void *write_to_server(void *argv)
{
  int sockfd =*(int *)argv;

  char *write_buf =NULL;
  ssize_t send_count;

  write_buf=malloc(sizeof(char)*1024);

  if(!write_buf)
  {
    printf("写缓存申请异常,端开连接\n");
    shutdown(sockfd,SHUT_WR);

    perror("malloc client write_buf");

    return NULL;
  }
  while(fgets(write_buf,1024,stdin)!=NULL)
  {
    //按字符串实际长度发送; 固定发 1024 字节会把缓冲区里的残留数据一起发出去
    send_count= send(sockfd,write_buf,strlen(write_buf),0);
    if(send_count<0)
    {
      perror("send");

    }
  }
  printf("接收到命令行的终止信号，不再写入,关闭连接.....\n");
  shutdown(sockfd,SHUT_WR);
  free(write_buf);

  return NULL;
}
int main(void)
{
  int sockfd,temp_result;

  pthread_t pid_read,pid_write;

  struct sockaddr_in server_addr;

  memset(&server_addr,0,sizeof(server_addr));

  server_addr.sin_family=AF_INET;
  //连接本机  127.0.0.1
  server_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  //连接端口6666
  server_addr.sin_port=htons(6666);

  //创建socket
  sockfd=socket(AF_INET,SOCK_STREAM,0);

  handle_error("socket",sockfd);

  //连接server
  temp_result=connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
  handle_error("connct",temp_result);

  //启动一个子线程，用来读取服务端数据，并打印到stdout
  pthread_create(&pid_read,NULL,read_from_server,(void *)&sockfd);
  //启动一个子线程，用来从命令行读取数据并发送到服务端
  pthread_create(&pid_write,NULL,write_to_server,(void *)&sockfd);

  //主线程等待子线程退出
  pthread_join(pid_read,NULL);
  pthread_join(pid_write,NULL);

  printf("关闭资源\n");
  close(sockfd);

  return 0;

}
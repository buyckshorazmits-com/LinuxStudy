#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<errno.h>


#define  SERVER_PORT 6666
#define  BUFFER_SIZE 1024
#define  MAX_EVENTS  10

#define  handle_error(cmd,result)\
         if(result<0)            \
         {                       \
          perror(cmd);           \
          exit(EXIT_FAILURE);    \
         }                       


char *read_buf=NULL;
char *write_buf=NULL;

void init_buf()
{
  read_buf=malloc(sizeof(char ) *BUFFER_SIZE);

  //判断内存是否分配成功 
  if(!read_buf)
  {
    printf("服务端读缓存创建异常，断开连接\n");
    perror("malloc sever read_buf");
    exit(EXIT_FAILURE);
  }

  //判断内存是否分配成功
  write_buf=malloc(sizeof(char ) *BUFFER_SIZE);
  if(!write_buf)
  {
    printf("服务端写缓存创建异常，断开连接\n");
    free(read_buf);
    perror("malloc server write_buf");
    exit(EXIT_FAILURE);
  }

  memset(read_buf,0,BUFFER_SIZE);
  memset(write_buf,0,BUFFER_SIZE);
}

void clear_buf(char *buf)
{
  memset(buf,0,BUFFER_SIZE);

}

void set_nonblocking(int sockfd)
{
  int opts=fcntl(sockfd,F_GETFL);
  //读取文件操作符 在后面方便改变他的状态

  if(opts<0)
  {
    perror("fcntl(F_GETFL)");
    exit(EXIT_FAILURE);
  }

  opts |=O_NONBLOCK;

  int ret=fcntl(sockfd,F_SETFL,opts);

  if(ret<0)
  {
    perror("fcntl(F_SETFL)");
    exit(EXIT_FAILURE);
  }
}

int main(int argc , char const *argv[])
{
  init_buf();

  //声明sockfd,clientfd和函数返回状态变量
  int sockfd ,client_fd,temp_result;

  //声明服务端和客户端地址
  struct sockaddr_in server_addr,client_addr;

  memset(&server_addr,0,sizeof(server_addr));
  memset(&client_addr,0,sizeof(client_addr));

  //声明IPV4通信协议
  server_addr.sin_family=AF_INET;
  //绑定0.0.0.0地址，转换成网络字节序
  server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  //设置端口  不要设置特权端口
  server_addr.sin_port=htons(SERVER_PORT);

  //创建server socket

  sockfd=socket(AF_INET,SOCK_STREAM,0);
  handle_error("socket",sockfd);

  //绑定地址
  temp_result= bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
  handle_error("bind",temp_result);

  //进入监听模式
  temp_result=listen(sockfd,128);

  handle_error("listen",temp_result);
  //将sockfd设置为非阻塞模式
  set_nonblocking(sockfd);

  int epollfd,nfds;
  //epoll_event结构体   events:表示监视或要返回的事件类型  
  //      data: 用户数据 可以用来保存文件描述符或用户自定义数据
  struct  epoll_event ev,events[MAX_EVENTS];
  
  epollfd=epoll_create1(0);//创建一个epoll实例 返回一个相应的文件描述符
  handle_error("epoll_create",epollfd);
  ev.data.fd=sockfd;//登记监听 fd：贴标签 "sockfd"
  ev.events=EPOLLIN;//将监听连接的文件描述符 设置为可读事件

  temp_result=epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&ev);//将监听连接的socket 描述符添加到epoll
  handle_error("epoll_ctl",temp_result);

  socklen_t cliaddr_len =sizeof(client_addr);
  //接受client 连接
  while(1)
  {
    nfds=epoll_wait(epollfd,events,MAX_EVENTS,-1);
    //返回就绪的事件个数
    handle_error("epoll_wait",nfds);

    for(int i=0;i< nfds;i++)
    {
      if(events[i].data.fd == sockfd)//在就绪的事件里 判断那些是监听fd 的
      {
        client_fd=accept(sockfd,(struct sockaddr *)&client_addr,&cliaddr_len);

        handle_error("accept",client_fd);

        set_nonblocking(client_fd);
        printf("与客户端from %s at PORT %d 文件描述符 %d 建立连接 \n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),client_fd);

        ev.data.fd=client_fd;//登记客户端 fd ：贴上标签“client_fd"

        ev.events=EPOLLIN | EPOLLET;//设置对应的fd 文件描述符可读 设置为边沿触发 仅状态变化时 通知一次
        //这里的状态变化是指内核接受缓冲区 从空到有新数据到达或者客户端断开了连接
        epoll_ctl(epollfd,EPOLL_CTL_ADD,client_fd,&ev);
      }

      else if(events[i].events&EPOLLIN)//检测实际就绪事件中的可读事件
      {
        int count =0,send_count=0;
        client_fd=events[i].data.fd;//读取就绪的标签
        while((count =recv (client_fd,read_buf,BUFFER_SIZE,0))>0)
        {
          printf("receive message from client_fd : %d : %s \n",client_fd,read_buf);
          clear_buf(read_buf);

          strcpy(write_buf,"receive ~\n");
          send_count= send(client_fd,write_buf,strlen(write_buf),0);

          handle_error("send",send_count);
          clear_buf(write_buf);

        }

        if(count==-1&&errno ==EAGAIN)
        {
          printf("来自客户端client_fd : %d 当前批次的数据已经读取完毕，继续监听文件描述符集 \n",client_fd);

        }
        else if(count==0)
        {
          printf("客户端 client_fd: %d 请求关闭连接......\n",client_fd);

          strcpy(write_buf,"receive your shutdown signal \n");
          send_count=send(client_fd,write_buf,strlen(write_buf),0);

          handle_error("send",send_count);
          clear_buf(write_buf);

          //从epoll文件描述符集中移除client_fd

          printf("从epoll文件描述符集中移除client_fd: %d \n",client_fd);
          epoll_ctl(epollfd,EPOLL_CTL_DEL,client_fd,NULL);

          printf("释放client_fd : %d 资源\n",client_fd);
          shutdown(client_fd,SHUT_WR);

          close(client_fd);
        }
      }
    }
  }

    printf("释放资源 \n");
    close(epollfd);
    close(sockfd);
    free(read_buf);
    free(write_buf);
  
    return 0;
}
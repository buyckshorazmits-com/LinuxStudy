#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>

#define handle_error(cmd,result) \
if(result<0)                     \
{                                \
  perror(cmd);                   \
  return -1;                     \
}

void *read_from_client_then_write(void *argv)
{
  int client_fd=*(int *)argv;
  //主线程把 fd 放在堆上传过来, 这里取完值就把这块内存还给系统, 防止内存泄漏
  free(argv);

  ssize_t count=0,send_count=0;
  char *read_buf=NULL;
  char *write_buf=NULL;

  read_buf =malloc(sizeof(char) *1024);
  //判断内存是否分配成功
  if(!read_buf)
  {
    printf("服务器度环村创建异常，断开连接\n");
    shutdown(client_fd,SHUT_WR);
    close(client_fd);
    perror("malloc sever read_buf");

    return NULL;
  }
  //判断内存是否分配成功
  write_buf=malloc(sizeof(char) *1024);
  if(!write_buf)
  {
    printf("服务端写缓存创建异常，端开连接\n");
    free(read_buf);
    shutdown(client_fd,SHUT_WR);
    close(client_fd);
    perror("malloc server write_buf");

    return NULL;

  }

  //只收 1023 字节, 给 '\0' 留位置; 出错(-1)也不能再当成真值继续循环
  while((count=recv(client_fd,read_buf,sizeof(char)*1024-1,0))>0)
  {
    //recv 不补结束符, printf("%s") 依赖 '\0', 必须自己补
    read_buf[count]='\0';
    printf("receive message from client_fd: %d :%s\n",client_fd,read_buf);
    strcpy(write_buf,"received~\n");
    //按实际长度发送, 避免把缓冲区里的残留数据一起发到网络上
    send_count=send(client_fd,write_buf,strlen(write_buf),0);
    if(send_count<0)
    {
      perror("send");
    }
  }
  if(count<0)
  {
    perror("recv");
  }

  printf("客户端client_fd: %d 请求关闭连接.......\n",client_fd);
  strcpy(write_buf,"receive your shutdown signal \n");

  send_count=send(client_fd,write_buf,strlen(write_buf),0);
  if(send_count<0)
  {
    perror("send");
  }
  printf("释放client_fd:%d 资源\n",client_fd);

  shutdown(client_fd,SHUT_WR);
  close(client_fd);
  free(read_buf);
  free(write_buf);

  return NULL;

}

int main(void)
{
  int sockfd,temp_result;
  struct sockaddr_in server_addr,client_addr;

  memset(&server_addr,0,sizeof(server_addr));//将server_addr结构体清零
  memset(&client_addr,0,sizeof(client_addr));//将client_addr结构体清零

  //声明IPV4通信协议
  server_addr.sin_family=AF_INET;
  //绑定0.0.0.0地址，转换成网络字节序后完成设置
  server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  //端口随便用一个，不要使用特权端口
  server_addr.sin_port = htons(6666);

  //创建server socket
  sockfd =socket(AF_INET,SOCK_STREAM,0);//socket() 的作用是向操作系统申请创建一个用于网络通信的套接字。
  //没有创建套接字，程序就没有对应的通信端点，无法通过这个套接字调用后续的网络通信接口。没有套接字程序就没有对应的通信节点 就进行不了通信。
  handle_error("socket",sockfd);

  //绑定地址
  temp_result=bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));//bind() 的作用是把指定的本地地址与套接字关联起来。
  handle_error("bind",temp_result);

  //进入监听模式
  temp_result =listen(sockfd,128);
  handle_error("listen",temp_result);

  socklen_t cliaddr_len =sizeof(client_addr);//创建一个变量 cliaddr_len，记录 client_addr 这个客户端地址结构体占用的内存大小，供后面的 accept() 函数使用。
  //接受client连接
  while (1)
  {
    //accept 可能改写 cliaddr_len, 所以每轮都重新赋值
    cliaddr_len = sizeof(client_addr);
    int client_fd=accept(sockfd,(struct sockaddr *)&client_addr,&cliaddr_len);
    handle_error("accpet",client_fd);

    printf("与客户端from %s at PORT %d 文件描述符 %d 建立连接\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),client_fd);

    pthread_t  pid_read_write;
    //启动一个子线程，用来读取客户端数据，并打印到stout
    //注意，此处的pid_read_write不是线程id,而是用于线程处理函数的标识符

    //client_fd 是循环体里的局部变量, 每轮 accept 都会改写同一块栈内存;
    //如果直接传 &client_fd, 线程真正取值的时机由调度器决定, 可能已经被下一轮覆盖
    //(上一轮的那个变量甚至已经出了作用域), 所以先复制一份到堆上, 线程用完自己 free
    int *p_client_fd = malloc(sizeof(int));
    if(!p_client_fd)
    {
      perror("malloc p_client_fd");
      close(client_fd);
      continue;
    }
    *p_client_fd = client_fd;

    if(pthread_create(&pid_read_write, NULL, read_from_client_then_write, (void *)p_client_fd))
    {
      perror("pthread_create");
      close(client_fd);  //线程没启动, 这条连接只能由主线程来关
      free(p_client_fd);//这块内存也要回收
      continue;
    }
    //将子进程处理为detached状态，使其终止是自动回收资源，同时不阻塞主线程
    
    pthread_detach(pid_read_write);
    printf("创建子线程并处理为detached状态\n");

  }
  printf("释放资源\n");
  close(sockfd);
  return 0;
}
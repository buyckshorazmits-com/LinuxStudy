//服务端基于多进程的支持多个连接的范例程序
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include<signal.h>


#define handle_error(cmd,result)  \
if(result<0)                      \
{                                 \
  perror(cmd);                    \
  return -1;                      \
}

void zombie_dealer(int sig)
{
  pid_t pid ;
  int status;
  //一个SIGCHLD可能对应多个子进程的退出，
  //使用while循环回收所有退出的子进程，避免僵尸进程的出现
  while((pid = waitpid(-1,&status,WNOHANG))>0)
  {
    if(WIFEXITED(status))//子进程正常结束
    {
      printf("子进程：%d 以%d 状态正常退出，已被回收\n",pid,WEXITSTATUS(status));
    }
    else if(WIFSIGNALED(status))//子进程被信号异常终止
    {
      printf("子进程：%d 被%d 信号杀死，已被收回\n",pid,WTERMSIG(status));
    }
    else
    {
      printf("子进程：%d因其他原因退出，已被回收\n",pid);
    }
  }
}
void  *read_from_client_then_write(void *argv)
 {
  int client_fd = *(int *)argv;

  ssize_t count=0 ,send_count=0;
  char *read_buf =NULL;
  char *write_buf=NULL;

  read_buf=malloc(sizeof(char)* 1024);
  //判断内存是否分配成功

  if(!read_buf)
  {
    printf("服务端pid:  %d :读缓存创建异常，断开连接\n",getpid());
    shutdown(client_fd,SHUT_WR);//关闭写方向（发送）
    //客户端的send() ->服务端的recv()  写方向
    //客户端的recv() ->服务端的send()  读方向
    close(client_fd);
    perror("malloc sever read_buf");
    return NULL;
  }

  //判断内存是否分配成功
  write_buf=malloc(sizeof(char)* 1024);

  if(!write_buf)
  {
    printf("服务端pid:  %d :写缓存创建异常，断开连接\n",getpid());
    free(read_buf);
    shutdown(client_fd,SHUT_WR);
    close(client_fd);
    perror("malloc server write_buf");
    return NULL;
  }

  //recv 返回值: >0 收到的字节数, 0 对端关闭, -1 出错
  //写 1024 就留不出结束符的位置, 必须只收 1023 字节
  //另外原写法 while((count=recv(...))) 在 recv 返回 -1 时是真值, 会死循环刷屏
  while((count=recv(client_fd,read_buf,1024-1,0))>0)
  {
    //recv 不会自动补结束符, 而下面的 printf("%s") 依赖 '\0'
    //不补的话会一直往后读到堆里的脏数据(未定义行为)
    read_buf[count]='\0';

    printf("服务端pid: %d :receive message from client_fd: %d : %s \n",getpid(),client_fd,read_buf);

    sprintf(write_buf,"服务端pid: %d : received~\n",getpid());
    //按实际长度发送; 固定发 1024 字节会把 malloc 后未初始化的残留数据一起发到网络
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

  printf("服务端pid : %d :客户端client_fd: %d 请求关闭连接......\n",getpid(),client_fd);
  sprintf(write_buf,"服务端pid: %d : receive your shutdown signal\n",getpid());
  //同样按实际长度发送
  send_count=send(client_fd,write_buf,strlen(write_buf),0);
  if(send_count<0)
  {

    perror("send");
  }

   printf("服务端pid: %d : 释放client_fd:%d 资源\n",getpid(),client_fd);
   shutdown(client_fd,SHUT_WR);
   close(client_fd);
   free(read_buf);
   free(write_buf);

   return NULL;
  
}

int main(int argc, char const *argv[])
{
  int sockfd,temp_result;
  struct sockaddr_in server_addr,client_addr;

  memset(&server_addr,0,sizeof(server_addr));
  memset(&client_addr,0,sizeof(client_addr));

  //声明IPV4的通信协议 
  server_addr.sin_family=AF_INET;
  //绑定0.0.0.0地址，转换成网络字节序后完成设置
  server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  //端口随便用一个，但是不要使用特权端口
  server_addr.sin_port=htons(6666);

  //创建server socket
  sockfd=socket(AF_INET,SOCK_STREAM,0);
  handle_error("socket",sockfd);

  //绑定server socket
  temp_result=bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
  handle_error("bind",temp_result);
   //服务端进入监听模式
   temp_result= listen(sockfd,128);
   handle_error("listen",temp_result);

   socklen_t cliaddr_len= sizeof(client_addr);
   
   //注册信号处理函数，处理SIGCHLD信号，避免僵尸进程的出现
  signal(SIGCHLD,zombie_dealer);

  //对端已关闭时再 send 会收到 SIGPIPE, 默认动作是杀死该进程
  //忽略它之后 send 返回 -1 并置 errno=EPIPE, 由代码自行处理
  signal(SIGPIPE,SIG_IGN);

   //接受客户端连接
   while(1)
   {
    //cliaddr_len 是"传入-传出"参数: 进 accept 时必须是缓冲区大小,
    //accept 返回时会被改成实际地址长度。放在循环外不复位, 第二次起就是脏值
    cliaddr_len= sizeof(client_addr);
    int client_fd=accept(sockfd,(struct sockaddr *)&client_addr,&cliaddr_len);
    handle_error("accpet",client_fd);

    pid_t pid=fork();

    if(pid>0)
    {
      //进入父进程
      printf("this is father ,pid is %d ,continue accepting.....\n",getpid());

      //父进程不需要处理 client_fd, 释放文件描述符，使其引用计数减1,方便子进程释放client_fd资源，其引用计数可以减到零，从而释放资源
      close(client_fd);

    }
    else if(pid==0)
    {
      close(sockfd);
      printf("与客户端 from %s at PORT %d 文件描述符 %d 建立连接\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),client_fd);

      printf("新的服务端 pid 为： %d\n",getpid());
      //读取客户端数据，并打印到屏幕上 stdout
      read_from_client_then_write((void *)&client_fd);

      //释放资源并终止子进程
      close(client_fd);
      exit(EXIT_SUCCESS);
    }
    else
    {
      //fork 失败(pid<0): 父进程里这个 client_fd 没人接手, 必须自己关掉, 否则 fd 泄漏
      perror("fork");
      close(client_fd);
      continue;
    }

   }

   printf("释放资源\n");

   close(sockfd);

   return 0;
}
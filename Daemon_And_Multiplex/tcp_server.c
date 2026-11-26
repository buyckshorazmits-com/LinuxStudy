#include<sys/socket.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include<signal.h>
#include<syslog.h>

int sockfd;

void zombie_dealer(int sig)
{
  pid_t pid ;
  int status;
  char buf[1024];

  memset(buf,0,1024);
  //一个SIGCHLD可能对应多个子进程的退出
  //使用while循环回收所有退出的子进程，避免僵尸进程的出现
  while((pid = waitpid(-1,&status,WNOHANG))>0)//Wait NO HANG：非阻塞  没有已退出子进程时立刻返回0,不会把信号处理函数卡住
  {
    if(WIFEXITED(status))
    {
      sprintf(buf,"子进程： %d 以%d 状态正常退出,已被回收\n",pid ,WEXITSTATUS(status));
      syslog(LOG_INFO,"%s",buf);

    }
    else if(WIFSIGNALED(status))
    {
      sprintf(buf,"子进程：%d 被 %d信号杀死,已被回收\n",pid,WTERMSIG(status));
      syslog(LOG_INFO,"%s",buf);
      
    }
    else  
    {
      sprintf(buf,"子进程：%d 因为其他原因退出，已被回收\n",pid);
      syslog(LOG_WARNING,"%s",buf);
    }
  }
}

void sigterm_handler(int sig)
{
  syslog(LOG_NOTICE,"服务端接收到守护进程发出的SIGTERM,准备退出...");
  syslog(LOG_NOTICE,"释放sockfd");
  close(sockfd);
  syslog(LOG_NOTICE,"释放syslog连接,服务端进程终止");

  closelog();

  //退出
  exit(EXIT_SUCCESS);

}

void read_from_client_then_write(void *argv)
{
  int client_fd=*(int *)argv;

  ssize_t count=0,send_count=0;
  char *read_buf=NULL;
  char *write_buf=NULL;

  char log_buf[1024];
  memset(log_buf,0,1024);

  read_buf=malloc(sizeof(char) *1024);

  //判断内存是否分配成功
  if(!read_buf)
  {
    sprintf(log_buf,"服务端pid: %d :读缓存异常，断开连接\n",getpid());
    syslog(LOG_ERR,"%s",log_buf);
    shutdown(client_fd,SHUT_WR);

    close(client_fd);

    return ;
  }

  //判断内存是否分配成功
  write_buf=malloc(sizeof(char) * 1024);

  if(!write_buf)
  {
   sprintf(log_buf,"服务端pid: %d :写缓存创建异常，断开连接\n",getpid());
   syslog(LOG_ERR,"%s",log_buf);

   free(read_buf);
   shutdown(client_fd,SHUT_WR);

   close(client_fd);

   return ;
  }

  // recv 返回 >0 才是真正读到数据；<0 是出错，0 是对端关闭写端
  while((count=recv(client_fd,read_buf,1023,0))>0)
  {
    //count==0 时表示对端 写端关闭 表示对端不能数据
    read_buf[count]='\0';//recv 不会自动补字符串结束符，不补会影响后面的 %s/syslog

    sprintf(log_buf,"服务端pid: %d :receive message from client_fd: %d :%s",getpid(),client_fd,read_buf);
    syslog(LOG_INFO,"%s",read_buf);
    memset(log_buf,0,1024);

    sprintf(write_buf,"服务端pid: %d :received~\n",getpid());
    send_count=send(client_fd,write_buf,strlen(write_buf),0);//发往客户端内容

  }
  if(count<0)
  {
    syslog(LOG_ERR,"server recv error");
  }

    sprintf(log_buf,"服务端pid: %d :客户端client_fd:%d 请求关闭连接 ......\n",getpid(),client_fd);
    syslog(LOG_NOTICE,"%s",log_buf);

    sprintf(write_buf,"服务端pid: %d :receive your shutdown signal\n",getpid());

    send_count=send(client_fd,write_buf,strlen(write_buf),0);

    sprintf(log_buf,"服务端pid : %d :释放client_fd: %d资源\n",getpid(),client_fd);

    syslog(LOG_NOTICE,"%s",log_buf);
    shutdown(client_fd,SHUT_WR);
    close(client_fd);
    free(read_buf);
    free(write_buf);
    
    return ;

}

int main(int argc,char const *argv[])
{

  int temp_result;
  struct sockaddr_in server_addr,client_addr;

  memset(&server_addr,0,sizeof(server_addr));
  memset(&client_addr,0,sizeof(client_addr));

  //声明使用IPv4协议
  server_addr.sin_family=AF_INET;
  //绑定0.0.0.0地址，转换成网络字节序后完成设置
  server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  server_addr.sin_port=htons(6666);

  //创建server socket
  sockfd=socket(AF_INET,SOCK_STREAM,0);

  //绑定地址
  temp_result=bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
  //进入监听模式
  temp_result=listen(sockfd,128);

  socklen_t cliaddr_len=sizeof(client_addr);

  //注册信号处理函数，处理SIGHLD信号，避免僵尸进程出现
  signal(SIGCHLD,zombie_dealer);
  //处理SIGTERM函数，以优雅退出
  signal(SIGTERM,sigterm_handler);

  char log_buf[1024];

  memset(log_buf,0,1024);
  //接受clinet 连接

  while (1)
  {
    int client_fd =accept(sockfd,(struct sockaddr *)&client_addr,&cliaddr_len);

    pid_t pid =fork();
    if(pid >0)
    {
      sprintf(log_buf,"this is father,pid is %d,continue accepting...\n",getpid());
      syslog(LOG_INFO,"%s",log_buf);

      memset(log_buf,0,1024);
      //附近成不需要处理 client_fd,释放文件描述符，使其引用计数减一，以便子进程释放client_fd后，其引用计数可以减为0,从而释放资源
      close(client_fd);
    }

    else if(pid==0)
    {
      //子进程不需要处理sockfd,释放文件描述符，使其引用计数减一
      close(sockfd);
      sprintf(log_buf,"与客户端from %s at PORT %d 文件描述符 %d 建立连接\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),client_fd);

      syslog(LOG_INFO,"%s",log_buf);

      memset(log_buf,0,1024);
      
      sprintf(log_buf,"新的服务端pid为: %d\n",getpid());
      syslog(LOG_INFO,"%s",log_buf);
      memset(log_buf,0,1024);

      //读取客户端数据，并打印到stdout
      read_from_client_then_write((void *)&client_fd);

      //释放资源并终止子进程
      close(client_fd);
      exit(EXIT_SUCCESS);

    }
  }
  
}
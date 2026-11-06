#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>
// 客户端基于多进程的范例程序

#define handle_error(cmd,result)\
  if(result<0)                  \
  {                             \
    perror(cmd);                \
    return -1;                  \
  }

  void *read_from_server(void *argv)
  {
    int sockfd=*(int *)argv;
    char *read_buf =NULL;
    ssize_t  count=0;

    read_buf =malloc(sizeof(char)* 1024);
    if(!read_buf)
    {
      perror("malloc client read_buf");
      return NULL;
    }

    //recv 返回值语义: >0 收到的字节数, 0 对端关闭, -1 出错
    //注意括号不能省: 写成 count=recv(...)>0 时, '>' 优先级高于 '=',
    //会被解析成 count=(recv(...)>0), count 只剩 0/1,
    //于是 read_buf[1]='\0' 只输出一个字节(汉字还会被砍成半个)
    //只收 1023 字节, 给结束符 '\0' 留出位置
    while((count=recv(sockfd,read_buf,sizeof(char)*1024-1,0))>0)
    {
      //recv 不会自动补结束符, 而 fputs 依赖 '\0'
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
    int sockfd=*(int *)argv;
    char *write_buf =NULL;
    ssize_t send_count;

    write_buf=malloc(sizeof(char)*1024);
    if(!write_buf)
    {
      printf("写缓存申请异常，断开连接\n");
      shutdown(sockfd,SHUT_WR);
      perror("malloc client write_buf");

      return NULL;

    }

    while(fgets(write_buf,1024,stdin)!=NULL)
    {
      //按字符串实际长度发送; 固定发 1024 字节会把 malloc 后未初始化的残留数据发出去
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

    //对端已关闭时再 send 会收到 SIGPIPE, 默认动作是杀死整个进程
    //忽略它之后 send 会返回 -1 并置 errno=EPIPE, 由代码自行处理
    signal(SIGPIPE,SIG_IGN);

    server_addr.sin_family=AF_INET;
    //连接到本机 127.0.0.1
    server_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK); 
    //配置到端口6666
    server_addr.sin_port=htons(6666);
    
    //创建socket

    sockfd= socket(AF_INET,SOCK_STREAM,0);

    handle_error("socket",sockfd);
    //连接到server端
    temp_result=connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    handle_error("connect",temp_result);

    //启动子线程读取服务端数据 并打印到stdout
    //注意传的是 &sockfd(地址), 不是 sockfd(值):
    //线程函数里是 *(int *)argv, 若传 (void *)sockfd 相当于访问地址 0x4, 立刻段错误
    //sockfd 是 main 的局部变量, 它活到 pthread_join 之后才 close, 所以地址一直有效
    if(pthread_create(&pid_read,NULL,read_from_server,(void *)&sockfd))
    {
      perror("pthread_create read");
    }
    //启动子线程从命令行中读取数据 并发送到服务端
    if(pthread_create(&pid_write,NULL,write_to_server,(void *)&sockfd))
    {
      perror("pthread_create write");
    }
 
    //主线程等待子进程退出
    pthread_join(pid_read,NULL);
    pthread_join(pid_write,NULL);

    printf("关闭资源\n");

    close(sockfd);

    return 0;

  }
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>

#define handle_error(cmd,result)\
     if(result<0)               \
     {                          \
      perror(cmd);              \
      return -1;                \
     }

int main(int argc,char const *argv[])
{
  int sockfd,temp_result,client_fd;
  struct sockaddr_in server_addr;
  char *buf =malloc(1024);

  memset(&server_addr,0,sizeof(server_addr));

  //定义IPv4结构体
  //声明IPV4通信
  server_addr.sin_family=AF_INET;
  //绑定本地回环地址，准还成网络字节序后完成设置
  server_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);//配置服务端的目标IP和端口INADDR_LOOPBACK=127.0.0.1
  //端口随便用一个   不要使用特权端口
  server_addr.sin_port=htons(6666);


  //127.0.0.1:临时端口
  //通常由操作系统在首次 sendto() 时自动分配，例如 53000

  //创建server socket 注意通信类型
  sockfd = socket(AF_INET,SOCK_DGRAM,0);//SOCK_DGRAM 使用UDP  SOCK_STREAM使用TCP
  handle_error("socket",sockfd);

  do
  {
    write(STDOUT_FILENO,"Type  something you want to send:",34);
    //标准输入读取数据
    int buf_len =read(STDIN_FILENO,buf,1023);

    temp_result=sendto(sockfd,buf,buf_len,0,(struct sockaddr *)&server_addr ,sizeof(server_addr));//目标服务器地址 (struct sockaddr *)&server_addr
    handle_error("sendto",temp_result);

    //清空缓冲区
    memset(buf,0,1024);
    //尝试接受数据
    temp_result=recvfrom(sockfd,buf,1024,0,NULL,NULL);//等待服务器回复
    handle_error("recvfrom",temp_result);
    //如果服务端发来的不是EOF

    if(strncmp(buf,"EOF",3)!=0)//将缓冲区的内容与EOF相比
    {
      //inet_ntoa() 将IPV4 地址转换成点分十进制字符串   ntohs()将网络字节序转换成主机字节序
      printf("received msg from %s at port %d :%s",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port),buf);
      //这里相当于说明了  服务端的地址信息 IP地址与端口
    }
  } while (strncmp(buf,"EOF",3)!=0);

  free(buf);

  return 0;
  

}
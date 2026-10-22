#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
//演示“主机字节序”和“网络字节序”之间的相互转换，也就是 socket 编程里 htons / ntohs 的用法。
int main( int argc,char const *argv[])
{
    //网络地址赋值
  unsigned short local_num=0x1F,network_num=0;
  //host to network short，把 16 位整数从本机字节序转成网络字节序（大端）。端口号是 16 位，所以用 htons
  network_num=htons(local_num);

  printf("将主机的字节序的无符号整数：0x%hx转换成网络字节序的结果为：0x%hx\n",local_num,network_num);
//这里的则是反过来的
  local_num=ntohs(network_num);
  printf("将网络字节序的无符号整数：0x%hx 转换成主机的字节序结果：0x%hx\n",network_num,local_num);

  return 0;

}
//同一个 16 位数在不同字节序机器上的内存排列不同；x86 是小端，而网络协议规定用大端（网络字节序）。
//跨主机传输整数（端口、长度、自定义协议字段）前必须统一成网络字节序；接收端再转回主机序。

//0x1f=0x001F  
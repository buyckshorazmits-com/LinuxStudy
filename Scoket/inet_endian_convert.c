#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

/*
 * 本文件演示 IPv4 地址在几种表示形式之间的转换：
 *   "192.168.6.101"（点分十进制字符串）
 *   in_addr_t / struct in_addr（32 位网络字节序地址）
 * 以及 inet_addr / inet_aton / inet_pton / inet_ntoa /
 * inet_lnaof / inet_netof / inet_makeaddr 的用法。
 *
 * 关键结论：
 *   1. inet_addr / inet_aton / inet_pton / inet_makeaddr 产生的地址
 *      都是“网络字节序”（大端）。
 *   2. x86 是小端，用 %X 直接打印 s_addr 会看到字节顺序反过来：
 *      192.168.6.101 的网络序字节是 C0 A8 06 65，
 *      在 x86 上按 32 位整数打印得到 0x6506A8C0；
 *      想看到 0xC0A80665，需要先 ntohl() 或按字节打印。
 *   3. 实际项目建议用 inet_pton / inet_ntop；
 *      inet_lnaof / inet_netof / inet_makeaddr 属于过时接口，仅作学习。
 */

int main(int argc,char const *argv[])
{
  // ========== 1. 准备三个容器，分别存放三种形式的地址 ==========
  struct sockaddr_in server_addr;     // 套接字地址结构，这里只用到它的 sin_addr 字段
  struct in_addr     server_in_addr;  // 只存放 4 字节 IP 的结构体
  in_addr_t          server_in_addr_t;// 32 位整数形式的 IP

  // 清零，避免使用/打印到未初始化的内容
  memset(&server_addr,0,sizeof(server_addr));
  memset(&server_in_addr,0,sizeof(server_in_addr));
  memset(&server_in_addr_t,0,sizeof(server_in_addr_t));

  // ========== 2. 对照：手工把 4 个字节分别写成十六进制 ==========
  // 192 -> 0xC0, 168 -> 0xA8, 6 -> 0x06, 101 -> 0x65
  // 拼起来就是 0xC0A80665，也就是 192.168.6.101 在网络上的 4 个字节
  printf("192.168.6.101的16进制表示为0x%X 0x%X 0x%X 0x%X \n",192,168,6,101);

  // ========== 3. inet_addr：字符串 -> in_addr_t（网络字节序）==========
  // 不推荐使用：出错时返回 INADDR_NONE(0xFFFFFFFF)，
  // 但 255.255.255.255 本身也是合法地址，无法区分“出错”和“有效地址”
  server_in_addr_t =inet_addr("192.168.6.101");
  // 小端机器上直接打印会显示 0x6506A8C0（字节反了）；
  // 改成 ntohl(server_in_addr_t) 才能得到 0xC0A80665
  printf("inet_addr convert:0x%X\n",server_in_addr_t);

  // ========== 4. inet_aton：字符串 -> struct in_addr（网络字节序）==========
  // 成功返回非 0，失败返回 0
  inet_aton("192.168.6.101",&server_in_addr);
  printf("inet_aton convert: 0x%X\n", server_in_addr.s_addr);

  // ========== 5. inet_pton：现代接口，字符串 -> 二进制地址（网络字节序）==========
  // AF_INET 表示 IPv4；成功返回 1，地址非法返回 0，参数错误返回 -1
  // 结果直接写入 sockaddr_in 的 sin_addr 字段
  inet_pton(AF_INET,"192.168.6.101",&server_addr.sin_addr);
  printf("inet_pton 后 server_addr.sin_addr 的16进制表示为0x%X\n",server_addr.sin_addr.s_addr);

  // ========== 6. inet_ntoa：struct in_addr（网络序）-> 字符串 ==========
  // 返回指向函数内部静态缓冲区的指针，下次调用会覆盖，且非线程安全；
  // 多线程环境应使用 inet_ntop
  printf("通过inet_ntoa打印inetpton 转化的地址：%s\n",inet_ntoa(server_addr.sin_addr));

  // ========== 7. inet_lnaof：取出主机号部分（返回值为主机字节序）==========
  // 192.168.6.101 是 C 类地址：网络号 24 位，主机号 8 位
  // 所以这里得到 0x65，即十进制的 101
  printf("local net sectoin:0x%X\n",inet_lnaof(server_addr.sin_addr));

  // ========== 8. inet_netof：取出网络号部分（返回值为主机字节序）==========
  // 192.168.6.0/24 的网络号是 0xC0A806
  printf("netowrd number sectoin； 0x%X \n",inet_netof(server_addr.sin_addr));

  // ========== 9. inet_makeaddr：网络号 + 主机号 -> struct in_addr（网络序）==========
  // 网络号沿用 192.168.6，主机号改成 102，拼出 192.168.6.102
  server_addr.sin_addr=inet_makeaddr(inet_netof(server_addr.sin_addr),102);

  // 返回值同样是网络字节序，小端机器上打印仍显示为 0x6606A8C0
  printf("inet_makeaddr :0x%X\n",server_addr.sin_addr.s_addr);

  // 用 inet_ntoa 验证拼接结果，应打印 192.168.6.102
  printf("通过inet_ntoa打印inet_makeaddr拼接后的地址%s \n",inet_ntoa(server_addr.sin_addr));

  return 0;
}
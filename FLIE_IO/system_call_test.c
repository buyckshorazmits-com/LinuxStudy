#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
int main(int argc, char const *argv[])
{
 /*
    const char *__path: 文件路径
    int __oflag: 用于指定打开文件的方式,可以是以下选项的组合:
        (1) O_RDONLY: 以只读方式打开文件 
        (2) O_WRONLY: 以只写方式打开文件 
        (3) O_RDWR: 以读写方式打开文件 
        (4) O_CREAT: 如果文件不存在,则创建一个新文件 
        (5) O_APPEND: 将所有写入操作追加到文件的末尾 
        (6) O_TRUNC: 如果文件存在并且以写入模式打开,则截断文件长度为0 
        还有其他标志,如O_EXCL（当与O_CREAT一起使用时,只有当文件不存在时才创建新文件）、O_SYNC（同步I/O）、O_NONBLOCK（非阻塞I/O）等 
    可选参数: mode -> 仅在使用了O_CREAT标志且文件尚不存在的情况下生效,用于指定新创建文件的权限位 权限位通常由三位八进制数字组成,分别代表文件所有者、同组用户和其他用户的读写执行权限
    return: (1) 成功时返回非负的文件描述符。
            (2) 失败时返回-1，并设置全局变量errno以指示错误原因。
*/


  int fd=open("io.text",O_RDONLY);
  if(fd==-1)
  {
    perror("open");
    exit(EXIT_FAILURE);

  }
  char buffer[1024];
  ssize_t bytes_read; /* 表示读取到的字节个数 */

  while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
  {
    write(STDOUT_FILENO, buffer, bytes_read);
  }
  if(bytes_read==-1)
  {
    perror("read");
    close(fd);
    exit(EXIT_FAILURE);
  }
  close(fd);
  return 0;
}


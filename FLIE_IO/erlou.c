#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char const *argv[])
{
  /*
        argc：
            表示一共收到了几个有效参数

        argv：
            保存每个参数的具体内容

        一楼传过来的是：

        args[0] = "/home/linux/vscoding/FLIE_IO/erlou"
        args[1] = "banzhang"
        args[2] = NULL

        所以二楼收到：

        argc = 2

        argv[0] = "/home/linux/vscoding/FLIE_IO/erlou"
        argv[1] = "banzhang"
    */
  if(argc<2)
  {

    /*
        因为下面要使用 argv[1]，

        所以必须保证至少有：

        argv[0]
        argv[1]

        也就是 argc 至少等于 2。
    */
    printf("参数不够\n");
    return 1;
  }
  printf("我是%s %d,可以跟着海哥上二楼\n",argv[1],getpid());
  
  return 0;
}

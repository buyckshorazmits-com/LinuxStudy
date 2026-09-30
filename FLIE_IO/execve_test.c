#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

    /*
        execve() 的作用：
        把当前进程正在运行的“一楼程序”，替换成“二楼程序”。

        注意：
        1. execve() 不会创建新的进程
        2. 切换前后 PID 不变，还是同一个进程
        3. execve() 成功后，一楼程序后面的代码不会继续执行
        4. execve() 失败时，才会返回 -1
    */
int main()
{
   /*exec系列函数  父进程跳转进入一个新进程
   推荐使用execve
    char *__path: 需要执行程序的完整路径名
    char *const __argv[]: 指向字符串数组的指针 需要传入多个参数
        (1) 需要执行的程序命令(同*__path)
        (2) 执行程序需要传入的参数
        (3) 最后一个参数必须是NULL
    char *const __envp[]: 指向字符串数组的指针 需要传入多个环境变量参数
        (1) 环境变量参数 固定格式 key=value
        (2) 最后一个参数必须是NULL
    return: 成功就回不来了 下面的代码都没有意义
            失败返回-1
    int execve (const char *__path, char *const __argv[], char *const __envp[]) 
    */

   char *name = "banzhang";
   printf("我是%s %d,我现在在一楼\n",name,getpid());
    char *argv[] = {"/home/linux/vscoding/FLIE_IO/erlou",NULL};

    /*
        args[]：准备传给二楼程序的参数

        args[0]：一般放要执行的程序名/程序路径
        args[1]：真正想传给二楼的数据，这里是 "banzhang"
        最后必须以 NULL 结束

        execve 成功后，二楼程序会收到：

        argc = 2

        argv[0] = "/home/linux/vscoding/FLIE_IO/erlou"
        argv[1] = "banzhang"
    */
   char *args[]={"/home/linux/vscoding/FLIE_IO/erlou",name,NULL};
    char *envs[]=
  {"PATH=/home/linux/work/tools/gcc_riscv32/bin:/home/linux/.local/bin:/home/linux/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin",NULL};

   int re=execve(args[0],args,envs);
   if(re==-1)
   {
    printf("你没机会上二楼\n");
    return -1;
   }
   return 0;
}
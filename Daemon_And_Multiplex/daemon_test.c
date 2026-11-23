//守护进程
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<syslog.h>
#include<string.h>
#include<fcntl.h>
#include<signal.h>
#include<errno.h>

pid_t pid;

int is_shutdown =0;

void signal_handler(int sig)
{
  switch(sig)
  {
    case SIGHUP:
     syslog(LOG_WARNING,"收到SIGHUP信号...");
     break;

    case SIGTERM:
     syslog(LOG_NOTICE,"接收到终止信号，准备退出守护进程...");
     syslog(LOG_NOTICE,"向子进程发送SIGTERM信号...");
     is_shutdown=1;
     kill(pid,SIGTERM);
    break;

    default:
    syslog(LOG_INFO,"Received unhandled signal");
  
  }
}
//守护进程的创建
void my_daemonize()
{
  pid_t pid ;

  pid =fork();//第一次fork()
  if(pid<0)
  {
    exit(EXIT_SUCCESS);
  }
  if(pid>0)
  {
    exit(EXIT_SUCCESS);//让原来的父进程退出，留下子进程  
    //这样是为了让后续创建的守护进程不再依赖原来的终端会话
  }

  if(setsid()<0)
  {
    //创建一个新的Session(会话)
    //创建一个新的Process Group(进程组)
    //父进程退出后 子进程变成进程组的组长
    exit(EXIT_SUCCESS);
  }
  //处理SIGHUP，SIGTERM信号

  signal(SIGHUP,signal_handler);
  signal(SIGTERM,signal_handler);

  //第一次fork()产生的是中间进程，调用setsid()创建新会话，第二次fork()创建的是守护进程
  //第二次创建的子进程 不会变成会话组的组长，进程组组长。从此无法再 open("/dev/tty") 拿到控制终端
  pid=fork();

  if(pid<0)
  {
    exit(EXIT_SUCCESS);
  }
  if(pid>0)
  {
   exit(EXIT_SUCCESS);
  }
  //umask(); chdir();初始化守护进程的环境
  //重置umask 文件权限掩码
  //守护进程"置为 0 / 清空"，含义是：不再受启动者环境的影响，创建文件时由代码显式写死权限位。

  umask(0);
  //将工作目录切换到根目录

  //避免占住挂载点
  //避免误写到启动目录
  chdir("/");

  //关闭所有打开的文件描述符
  for(int x=0;x<=sysconf(_SC_OPEN_MAX);x++)
  {
    close(x);
  }

  openlog("this is our daemonize process: ",LOG_PID,LOG_DAEMON);//建立并初始化系统日志记录环境，让守护进程能够把运行过程中的信息、错误和异常记录到 Linux 系统日志中，而不是输出到终端。
}

int main()
{
  my_daemonize();//在main()函数中调用my_daemonize() 是先将当前程序转换成一个真正的守护进程，在用守护进程启动和监控TCP服务端
  //提前创建一个守护进程

  while(1)
  {
    pid=fork();

    if(pid>0)//父进程主要负责去监控服务端的状态
    {
      syslog(LOG_INFO,"守护进程正在监听服务端进程....");
      waitpid(-1,NULL,0);//等待当前任意一个子进程退出，不是指定某一个PID的子进程
      
      if(is_shutdown)
      {
        syslog(LOG_NOTICE,"子进程已被回收,即将关闭syslog连接,守护进程退出");
        closelog();

        exit(EXIT_SUCCESS);
      }
      syslog(LOG_ERR,"服务端进程终止,3s秒后重启....");
      sleep(3);

    }
    else if(pid==0)
    {
      syslog(LOG_INFO,"子进程fork()成功");
      syslog(LOG_INFO,"启动服务端进程");

      char *path ="/home/linux/vscoding/Daemon_And_Multiplex/tcp_server";

      char *argv[]={"my_tcp_server",NULL};
      errno=0;
      execve(path,argv,NULL);
      //execve()失败才会执行后面的代码  成功了不会执行后面的代码
      char buf[1024];
      sprintf(buf,"errno: %d",errno);
      syslog(LOG_ERR,"%s",buf);

      syslog(LOG_ERR,"服务端进程启动失败");
      exit(EXIT_FAILURE);
    }

    else
    {
      syslog(LOG_ERR,"子进程fork()失败");
    }
  }
  return EXIT_SUCCESS;
}

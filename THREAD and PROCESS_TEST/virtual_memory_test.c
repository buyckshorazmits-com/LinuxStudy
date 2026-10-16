#include<stdio.h>
#include<unistd.h>

int main()
{
  int val=123;
  //定义变量接受子进程 

  __pid_t pid ;
  //创建一个子进程
  if((pid=fork())>0)
  {
    sleep(1);

    printf("父进程中的val=%d,val所在的地址为%d\n",val,&val);

  }
  else if(pid==0)
  {
    val=321;
    printf("子进程中的val=%d,val所在的地址为%d\n",val,&val);

  }
  else 
  {
    printf("子进程创建失败\n");
  }

  return 0;
}
//父子进程中各自打印了val变量的值和内存地址。子进程将val变量更改为321，我们特意让父进程休眠了一秒，确保子进程的更改先于父进程的输出。
//我们发现子进程修改了val变量的值，而父进程的val未受影响，说明两个进程的val变量是完全独立的。但是，二者的内存地址却完全相同。
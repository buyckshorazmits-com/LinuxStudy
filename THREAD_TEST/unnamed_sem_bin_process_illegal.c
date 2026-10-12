#include<stdio.h>
#include<unistd.h>
#include<semaphore.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/wait.h>
//在unnamed_sem_bin_process.c的基础上  改变sem_init()的配置   
int main()
{
  char *shm_sem_name="unnamed_sem_shm_sem";
  char *shm_name_value="unnamed_sem_shm_value";

  //创建共享内存对象
  int sem_fd=shm_open(shm_sem_name,O_CREAT | O_RDWR,0666);

  int value_fd=shm_open(shm_name_value,O_CREAT | O_RDWR,0666);

  //调整共享内存对象的大小
  ftruncate(value_fd,sizeof(int));

  ftruncate(sem_fd,sizeof(sem_t));

  //将内存对象映射到共享内存区域
  sem_t *sem=mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE,MAP_SHARED,sem_fd,0);

  int *value=mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,value_fd,0);
  // 初始化信号量和共享变量的值
  sem_init(sem,0,1);//pshared改成0  表示了同一个进程内的多个线程共享，而这里是两个进程之间共享内存  
                    //父进程和子进程虽然都拿到了 sem 的地址，但是不代表它们一定操作的是同一个“有效信号量状态”。
                    //这种用法违反了 POSIX 规范，行为是未定义的  可能引发了阻塞的问题
  *value=0;

  int pid=fork();

  if(pid>0)
  {
    sem_wait(sem);
    int tmp=*value+1;
    sleep(1);

    *value=tmp;
   sem_post(sem);
   //等待子进程执行完毕
   waitpid(pid,NULL,0);
   printf("this is father ,child was finished \n");
   printf("the final value ,value is %d \n",*value);

  }
  else if(pid==0)
  {
   sem_wait(sem);

   int tmp=*value +1;
   sleep(1);
   *value=tmp;

   sem_post(sem);

  }
  else
  {
    perror("fork");
  }
  //父进程执行到这里  子进程已经执行结束
  if(pid>0)
{
  if(sem_destroy(sem)==-1)
  {
    perror("sem_destory");
  }
}

//解除共享内存的映射，并关闭共享对象的文件描述符
if (munmap(sem,sizeof(sem_t))==-1)
{
  perror("munmap sem ");
}
if(munmap(value,sizeof(int))==-1)

{
  perror("munmap value");
}
if(close(sem_fd)==-1)
{
  perror("close sem_fd");
}
if(close(value_fd)==-1)
{
  perror("close value_fd");
}
 // 如果调用时别的进程仍在使用共享对象，则等待所有进程释放资源后，才会销毁相关资源。
    // shm_unlink只能调用一次,这里在父进程中调用shm_unlink
    if (pid > 0)
    {
        if (shm_unlink(shm_sem_name) == -1)
        {
            perror("father shm_unlink shm_sem_name");
        }

        if (shm_unlink(shm_name_value) == -1)
        {
            perror("father shm_unlink shm_value_name");
        }
    }


}
// mmap(MAP_SHARED)解决的是：多个进程看到同一块内存。
// sem_init(sem,1,1)解决的是：多个进程能够正确同步访问这块内存。
/*
    sem_init(sem, pshared, value)

    pshared参数用于指定信号量的共享范围：
    
    pshared = 0:
        表示该信号量用于同一个进程内部的线程之间共享。
        不适用于父子进程之间的同步。

    pshared = 1:
        表示该信号量用于多个进程之间共享。
        当信号量存放在共享内存中，并且父子进程通过mmap映射访问同一个
        sem_t对象时，必须设置为1。


    当前程序中：
        父进程通过fork()创建子进程，
        父子进程共同访问共享内存中的sem_t信号量，
        因此应该使用：

            sem_init(sem, 1, 1);


    如果错误设置为：

            sem_init(sem, 0, 1);

    则该信号量被定义为线程间同步信号量，
    用于进程间同步时行为是未定义的，可能出现以下情况：

    1. 正常情况（某些系统表现）
       父子进程可能仍然看到同一个信号量状态：
            初始:
                sem = 1
            父进程:
                sem_wait()
                sem = 0
                进入临界区

            子进程:
                sem_wait()
                发现sem=0
                阻塞等待

            父进程:
                sem_post()
                sem = 1
                唤醒子进程

    2. 互斥失效情况

       父子进程没有正确共享信号量状态：

            父进程:
                sem_wait()成功

            子进程:
                sem_wait()也成功

       两个进程同时进入临界区，
       导致共享变量被同时修改，
       出现竞态条件。

    3. 异常阻塞情况

       父子进程对信号量状态理解不一致：

            父进程认为:
                sem = 0

            子进程认为:
                sem = 0

       两个进程都在sem_wait()处等待，
       但是没有进程能够执行sem_post()释放信号量，
       最终造成死锁。


    因此：
        共享内存只能保证父子进程访问同一块数据，
        sem_init中的pshared参数决定信号量是否真正支持进程间同步。
*/
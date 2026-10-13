//在unnamed_sem_bin_process_illegal.c的基础上  改变sem_init（）的配置  是的进程之间能够对内存对象操作
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/mman.h>
#include<sys/types.h>

int main()
{
  char *shm_value_name="unnamed_sem_shm_value";
  
  //创建内存共享对象
  int value_fd=shm_open(shm_value_name,O_CREAT | O_RDWR,0666);

  //设置共享内存大小
  ftruncate(value_fd,sizeof(int));
  //将内存共享对象映射到共享内存区域
  int *value=mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,value_fd,0);

  sem_t sem;
  //初始化信号量和共享变量的值
  sem_init(&sem,1,1); //sem初始化 可以多个进程操作 
  //未将sem_t对象放入共享内存(mmap + MAP_SHARED)中，后面父子进程操作的sem 都是各个操作自己的  没有达到互斥的效果
  //导致在value值更新的时候  出现了丢更新的情况
  *value=0;

  int  pid=fork();
  if(pid>0)
  {
    sem_wait(&sem);
    int tmp=*value+1;
    sleep(1);
    *value=tmp;

    sem_post(&sem);

    //等待子进程完毕
    waitpid(pid,NULL,0);
    printf("this is father ,child was finished\n");
    printf("the final value is %d \n",*value);


  }else if(pid==0)
  {
    sem_wait(&sem);
    int tmp=*value + 1;
    sleep(1);
    *value=tmp;
    sem_post(&sem);

  }
  else
  {
    perror("fork");
  }
  //父进程执行到这里 子进程已经执行完毕  销毁信号量
  if(pid>0)
  {

    if(sem_destroy(&sem)==-1)
    {
      perror("sem destory");
    }

  }
  // 无论父子进程都应该解除共享内存的映射，并关闭共享对象的文件描述符
  if(munmap(value,sizeof(int))==-1)
  {
    perror("value munmap failed");
  }
  if(close(value_fd)==-1)
  {
  
    perror("close value_fd");

  }
 // 如果调用时别的进程仍在使用共享对象，则等待所有进程释放资源后，才会销毁相关资源。
    // shm_unlink只能调用一次,这里在父进程中调用shm_unlink

    if(pid>0)
    {
      if(shm_unlink(shm_value_name)==-1)
      {
        perror("shm_value_name unlink failed");
      }
    }


  return 0;
}
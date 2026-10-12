#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/wait.h>
//在unnamed_sem_bin_process_condition的基础上 添加信号量消除竟态条件的干扰

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
  sem_init(sem,1,1);
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
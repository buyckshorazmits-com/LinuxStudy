/*
 * ============================================================
 * named_sem_no_unlink.c
 * 有名信号量父子进程同步演示 —— 故意注释掉 sem_unlink 的版本
 * ============================================================
 * 运行顺序(按时间线):
 *
 *   1. 父进程 sem_open("/named_sem", O_CREAT, 0666, 0)
 *      创建有名信号量, 初值 0 (无许可, 谁来了都得等)。
 *      它实际是 /dev/shm/ 里的一个 32 字节内核对象,
 *      任何进程凭名字都能找到它。
 *
 *   2. fork() 分出子进程, 之后父子并行执行。
 *
 *   3. 父进程(pid>0): sem_wait 发现值为 0, 拿不到许可 -> 阻塞等待。
 *
 *   4. 子进程(pid==0): sleep(1) -> 打印 "this is son"
 *      -> sem_post 把值 0->1 并唤醒父进程 -> sem_close。
 *
 *   5. 父进程被唤醒: sem_wait 返回(拿走许可) -> 打印 "this is father"
 *      -> waitpid 回收已退出的子进程 -> sem_close。
 *      所以输出顺序固定为: 先 son, 后 father (信号量当"信号旗"用)。
 *
 *   6. 收尾: sem_unlink 被注释掉, 名字不删 =>
 *      /dev/shm/sem.named_sem 残留(32字节文件), 直到重启或手动 rm。
 *      下次运行 sem_open(O_CREAT) 会直接打开这个旧对象,
 *      初值 0 被忽略, 行为取决于残留的旧值:
 *        - 旧值=0 (正常跑完)   -> 看不出差别;
 *        - 旧值=1 (死在 post 之后) -> 父进程直接放行, 顺序颠倒。
 *      验证: ls -la /dev/shm/   清理: rm /dev/shm/sem.named_sem
 */

#include<stdio.h>
#include<unistd.h>
#include<semaphore.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/mman.h>

int main()
{
  char *sem_named="/named_sem";

  //初始化信号量

  sem_t *sem=sem_open(sem_named,O_CREAT,0666,0);

  pid_t pid=fork();

  if(pid>0)
  {
    sem_wait(sem);

    printf("this is father\n");

    //子进程执行完毕
    waitpid(pid,NULL,0);

    //释放引用
    sem_close(sem);
    //释放有名信号量
    /*if(sem_unlink(sem_named)==-1)
    {
      perror("sem_ulink");
    }
  */
   
  }

 else if(pid==0)
    {
      
      sleep(1);
      printf("this is son\n");
     
      sem_post(sem);
      //释放引用
      sem_close(sem);

    }
    else
    {
      perror("fork");
    }

   return 0;

}
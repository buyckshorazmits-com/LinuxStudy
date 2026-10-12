#include<stdio.h>
#include<unistd.h>
#include<semaphore.h>
#include<pthread.h>
/*============================================================================
  无名信号量实现"互斥"演示(二进制信号量, binary semaphore)
  场景: 1000 个线程, 每个线程把共享变量 share_num 加 1, 期望结果 = 1000

  为什么需要保护?
    share_num+1 实际是"读值 -> 加1 -> 写回"三步, 不是一步完成的原子操作;
    多个线程同时做, 可能互相覆盖(丢更新), 最终结果会小于 1000 —— 这就是竞态条件
    (不带保护的版本实测: 20 次里出现过 1 次 999)
在unnamed_sem_bin_thread_condition.c的基础上加入无名信号量解决竟态问题
  无名信号量怎么解决?
    sem_init(&unnamed_sem, 0, 1) 初始只发 1 个"许可"(相当于 1 把钥匙);
    线程进临界区前 sem_wait 领钥匙, 出来时 sem_post 还钥匙;
    → 同一时刻最多只有 1 个线程能进入临界区, 结果稳定为 1000
============================================================================*/

//无名信号量: 不靠名字, 靠这个变量的内存位置标识; 用于同一进程内的线程之间
sem_t unnamed_sem;
int share_num=0;        //共享变量: 1000 个线程都要给它 +1

void *plusOne(void *argv)
{
  //【第1步】P 操作(sem_wait): 尝试领走 1 个许可
  //  许可>0(比如1) → 减1, 顺利通过, 进入临界区
  //  许可=0        → 阻塞睡觉排队, 直到有人 sem_post 把它唤醒(睡觉不占CPU)
  sem_wait(&unnamed_sem);

  //【第2步】临界区: 只有领到钥匙的线程能执行
  //  读-加1-写回 必须整个被保护, 只保护一半同样会丢更新
  int tmp=share_num+1;
  share_num=tmp;

  //【第3步】V 操作(sem_post): 归还许可, 并唤醒一个正在排队的线程
  //  出去一个才能进来一个, 绝不会有 2 个线程同时在临界区
  sem_post(&unnamed_sem);

  return NULL;   //线程函数类型是 void*, 正常结束习惯返回 NULL
}
int main()
{
  //初始化无名信号量, 三个参数:
  //  ① &unnamed_sem 要初始化的信号量  ② 0=线程间共享(非0才是进程间共享)这个无名信号量供本进程内所有线程共用同一个对象。
  // ③ 初始许可数=1
  //  初始值 1 → 二进制信号量, 相当于一把互斥锁
  //  如果写成 0, 则所有线程第一时间全部堵死在 sem_wait(互相干等, 谁都不动)
  sem_init(&unnamed_sem,0,1);//同一时刻最多只有 1 个线程能通过 sem_wait 进入临界区，其余线程在 sem_wait 处阻塞排队，
  //睡死不占 CPU，直到有人 sem_post 还钥匙并唤醒下一个。

  pthread_t tid[1000];//保存1000个线程的ID
  //创建 1000 个线程, 每个线程都去执行 plusOne
  //  tid+i 即 &tid[i]: 把第 i 个线程的ID存到 tid[i] 里(注意不是 tid+1!)
  for(int i=0;i<1000;i++)
  {
    pthread_create(tid+i,NULL,plusOne,NULL);

  }

  //等待所有线程结束: 保证 1000 次 +1 全部完成, 才去读 share_num
  for(int i=0;i<1000;i++)
  {
    pthread_join(tid[i],NULL);
  }

   printf("share_num is %d\n", share_num);   //有信号量保护 → 稳定输出 1000
   sem_destroy(&unnamed_sem);                //所有线程用完, 销毁无名信号量
   return 0;
}
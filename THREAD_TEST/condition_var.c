#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
/*============================================================================
  线程间条件切换函数
  如果需要两个线程协同工作，可以使用条件变量完成线程切换。

  本程序是一个"生产者-消费者"模型:
    生产者线程 producer: 不断往缓冲区里放数据(白月光发送幸运数字)
    消费者线程 consumer: 不断从缓冲区里取数据(我收到幸运数字)

  两个工具配合使用:
    互斥锁 mutex : 保证同一时刻只有一个线程能读写 buffer/count,防止数据竞争
    条件变量 cond: 条件不满足时让线程先睡觉,条件可能满足时由对方把它叫醒
                   (不用反复轮询检查,省 CPU)

  使用条件变量的两条铁律:
    1. 检查条件要用 while 循环,不要用 if(被唤醒≠条件一定成立,醒来后必须重新检查)
    2. pthread_cond_wait 必须先拿到锁再调用;它返回时一定重新持有锁
============================================================================*/

#define BUFFER_SIZE 5          //缓冲区能放的数据个数
int buffer[BUFFER_SIZE];       //共享缓冲区,相当于一个货架
int count=0;                   //货架上当前有几个数据(0~BUFFER_SIZE),两个线程共享

//初始化互斥锁: 未加锁状态
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
//初始化条件变量: 两个方向共用这一个(谁在等就唤醒谁,本程序每个方向上最多只有一个等待者,够用)
static pthread_cond_t cond=PTHREAD_COND_INITIALIZER;


void *producer(void *arg)//生产者线程
{
  int item=1;
  while(1)//不断生产
  {
    //【第1步】拿锁: 要访问共享数据(buffer/count)前必须先拿锁,拿不到就在这里阻塞等待
    pthread_mutex_lock(&mutex);

    //【第2步】缓冲区满(count==BUFFER_SIZE)就睡觉,等消费者取走数据后叫醒我
    //  为什么用 while 而不是 if ?
    //  因为被唤醒后条件可能仍不满足(比如对方又抢先了一步),所以要醒来后重新检查,不满足就接着睡
    while(count==BUFFER_SIZE)
    {
      // pthread_cond_wait 一口气做了两件事:
      //   ① 释放锁 mutex(不释放的话消费者永远拿不到锁,谁都动不了 -> 死锁)
      //   ② 睡眠,直到被 signal/broadcast 唤醒
      // 被唤醒后它还要重新抢锁,抢到锁才会返回(wait 返回时手里一定拿着锁)
      pthread_cond_wait(&cond,&mutex);

    }
    //【第3步】运行到这里说明没满,可以写入: 放入一个数据,货架数量 count 加 1
    buffer[count++]=item++;
    printf("白月光发送一个幸运数字%d\n",buffer[count-1]);

    //【第4步】唤醒一个正在等待 cond 的线程(这里只可能是消费者): "有货了,快来取"
    //  注意: signal 只是"叫醒",并不释放锁! 对方被叫醒后也要抢到锁才能真正继续
    //  如果此刻没人等待,信号丢了也没关系: 条件(count)已经变了,对方下次检查时不会傻等
    pthread_cond_signal(&cond);//唤醒正在等待的一个线程  这里通知消费者可以使用数据

    //【第5步】释放锁,消费者这时才能拿到锁继续取数据
    pthread_mutex_unlock(&mutex);

  }

}
void *consumer(void *arg)//消费者线程
{
   while(1)//不断消费
   {

    //【第1步】拿锁: 访问共享数据前先拿锁
    pthread_mutex_lock(&mutex);

    //【第2步】货架空(count==0)就睡觉,等生产者放入数据后叫醒我
    //  同样用 while: 醒来后必须重新检查条件,可能又被对方抢先了(虚惊一场就接着睡)
    while(count==0)//如果缓冲区为空则等待生产者生产数据
    {
      //释放锁 + 睡眠; 被唤醒并重新拿到锁后才返回
     pthread_cond_wait(&cond,&mutex);

    }

    //【第3步】有数据了,取走一个: --count 是先减 1 再取(取走的是最后放进去的那个,像叠盘子)
    printf("我收到幸运数字了%d\n",buffer[--count]);

    //【第4步】唤醒一个正在等待 cond 的线程(这里只可能是生产者): "有空位了,可以继续生产"
    //  这一步不能省! 生产者可能正因货架满而睡着,不叫醒它,它就永远不会再醒来生产
    pthread_cond_signal(&cond);

    //【第5步】释放锁
    pthread_mutex_unlock(&mutex);
   }
}

int main()
{
  pthread_t pthread_producer,pthread_consumer;//保存两个线程的ID
  //创建两个线程: 第3个参数是线程入口函数; 创建成功立即返回,两个线程开始并发运行
  pthread_create(&pthread_producer,NULL,producer,NULL);//启动生产者线程
  pthread_create(&pthread_consumer,NULL,consumer,NULL);//启动消费者线程

  //join: 主线程在这里等待线程结束,顺便回收资源
  //注意: 两个线程都是 while(1) 死循环,永远不会结束 → 程序会一直运行,要用 Ctrl+C 手动停止
  pthread_join(pthread_producer,NULL);
  pthread_join(pthread_consumer,NULL);

  return 0;
}


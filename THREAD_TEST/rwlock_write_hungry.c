#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
//写饥饿测试
pthread_rwlock_t rwlock;
int shared_data = 0;
//在rwlock_rw_alternate.c的基础上，在读操作中添加1s的休眠。
//读写锁的写饥饿问题（Writer Starvation）是指在使用读写锁时，写线程可能无限期地等待获取写锁，
//因为读线程持续地获取读锁而不断地推迟写线程的执行。这种情况通常在读操作远多于写操作时出现。
void *lock_read(void *argv)
{
  pthread_rwlock_rdlock(&rwlock);
  printf("this is %s,shared_data= %d\n",(char*)argv,shared_data);

  sleep(1);
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}
void *lock_write(void *argv)
{
  pthread_rwlock_wrlock(&rwlock);
  int tmp=shared_data+1;
  shared_data=tmp;

  printf("this is %s,shared_data++\n",(char*)argv);
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}
int main()
{
  pthread_rwlock_init(&rwlock,NULL);
  pthread_t writer1,writer2,reader1,reader2,reader3,reader4,reader5,reader6;
  pthread_create(&writer1,NULL,lock_write,"writer1");
  pthread_create(&reader1,NULL,lock_read,"reader1");
  pthread_create(&reader2,NULL,lock_read,"reader2");
  pthread_create(&reader3,NULL,lock_read,"reader3");
  pthread_create(&writer2,NULL,lock_write,"writer2");
  pthread_create(&reader4,NULL,lock_read,"reader4");
  pthread_create(&reader5,NULL,lock_read,"reader5");
  pthread_create(&reader6,NULL,lock_read,"reader6");

  pthread_join(writer1,NULL);
  pthread_join(writer2,NULL);
  pthread_join(reader1,NULL);
  pthread_join(reader2,NULL);
  pthread_join(reader3,NULL); 
  pthread_join(reader4,NULL);
  pthread_join(reader5,NULL);
  pthread_join(reader6,NULL);

  pthread_rwlock_destroy(&rwlock);
}
//此时读操作总是连续执行的，且读操作休眠未结束时，写操作会被阻塞。
//与工作原理相符：① 读操作可以并发执行，相互之间不必争抢锁，多个读操作可以同时获得读锁；
//② 只要有一个线程持有读写锁，写操作就会被阻塞。我们在读操作
//中加了1s休眠，只要有一个读线程获得锁，在1s内写操作是无法执行的，其它读操作就可以有充足的时间执行，因此读操作就会连续发生，写操作必须等待所有读操作执行完毕方可获得读写锁执行写操作。
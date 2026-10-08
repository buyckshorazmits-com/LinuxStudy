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
  printf("this is %s,trying to rdlock\n",(char*)argv); //锁外打印：先后顺序不确定，便于观察各线程开始尝试的次序
  pthread_rwlock_rdlock(&rwlock);
  printf("this is %s,shared_data= %d\n",(char*)argv,shared_data);

  sleep(1);
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}
void *lock_write(void *argv)
{
  printf("this is %s,trying to wrlock\n",(char*)argv); //锁外打印：可观察写线程何时开始等待
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
  printf("main:writer1 created,main continues\n"); //pthread_create 立即返回，主线程未等待 writer1 执行
  pthread_create(&reader1,NULL,lock_read,"reader1");
  pthread_create(&reader2,NULL,lock_read,"reader2");
  pthread_create(&reader3,NULL,lock_read,"reader3");
  pthread_create(&writer2,NULL,lock_write,"writer2");
  pthread_create(&reader4,NULL,lock_read,"reader4");
  pthread_create(&reader5,NULL,lock_read,"reader5");
  pthread_create(&reader6,NULL,lock_read,"reader6");
  printf("main:all threads created,main goes to pthread_join\n"); //创建完所有线程后才进入 join 等待

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
//【运行观察与结论】
//0. pthread_create 创建线程后立即返回，主线程不会等线程函数执行完才执行下一条语句；
//   线程函数何时真正开始执行由调度器决定，因此 main 的打印与各线程的打印交错顺序不固定。
//1. 谁先获得锁并不确定：可能 writer1 先拿到写锁（此刻所有读线程被阻塞，写操作无休眠、很快结束），
//   也可能某个读线程先拿到读锁（此刻两个写线程都被阻塞）。
//2. 读写互斥规则是确定的：
//   ① 读锁是共享锁：多个读线程可以同时持有读锁，读操作可并发执行，彼此不必争抢；
//   ② 只要有一个读线程持有读锁，写操作就会被阻塞；
//   ③ 写锁是排他锁：写线程持锁期间，其它读、写线程全部阻塞。
//3. 写饥饿：glibc 的 pthread_rwlock 默认属性为 PTHREAD_RWLOCK_PREFER_READER_NP（读者优先）。
//   读操作中加了 1s 休眠，只要已有读线程持锁，新到来的读线程依然可以获得读锁（读者优先），
//   写线程必须等所有读线程都释放后才能拿到写锁；若读线程源源不断到来，写线程可能无限期等待，
//   这就是“写饥饿”。
//4. 因此“读操作连续执行、写操作被推迟到所有读操作完成之后”是本实验（读者优先+读多写少+读持锁1s）
//   下常见的观察现象；但线程调度与锁获取顺序本身并无固定保证（可多运行几次对比输出）。
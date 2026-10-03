#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
//读写操作执行顺序随机
pthread_rwlock_t rwlock;
int shared_data = 0;
//在rwlock_test.c的基础上做了如下修改：
//① 删除写操作的sleep()操作
//② 删除主线程中创建写线程之后的睡眠操作
//③ 将第二次写操作置于第三次读操作之后。
//这样做的目的是尽可能让读写操作间隔执行，但要注意的是，线程的执行顺序是由操作系统内核调度的，
//其运行规律并不简单地为“先创建先执行”。
//④ 运行结果显示，读写操作交替执行，读操作之间也有交替执行的情况。这种交错次序由内核调度器决定
//   （取决于各线程被调度上 CPU 的先后、锁等待队列的唤醒顺序），程序只保证读写锁的语义
//   （写独占、读共享），并不保证线程的执行先后，因此多次运行结果可能不同。
void *lock_read(void *argv)
{
  pthread_rwlock_rdlock(&rwlock);
  printf("this is %s,shared_data= %d\n",(char*)argv,shared_data);

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
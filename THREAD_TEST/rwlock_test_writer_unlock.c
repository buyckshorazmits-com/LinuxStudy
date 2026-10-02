#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
//读锁保护读取，允许多个读者并发；写锁保护修改，写者必须独占资源。
pthread_rwlock_t rwlock;
int share_data=0;

void *lock_reader(void *argv)
{
  pthread_rwlock_rdlock(&rwlock);//加读锁，允许多个读线程同时访问共享数据
  printf("this is%s ,value is %d \n",(char *)argv,share_data);

  pthread_rwlock_unlock(&rwlock);//关读锁
  return NULL;
}
void *lock_writer(void *argv)//反面示例：故意不加写锁，两个写者并发"读-改-写"，丢掉一次自增（最终 share_data=1 而非 2）
{
  //故意不加锁：sleep(1) 放大竞争窗口，使两个写者都先读到 0 再写回
  int tmp= share_data+1;

  sleep(1);
  share_data=tmp;
  printf("this is %s,share_data++\n",(char *)argv);
  return NULL;
}
int main()
{
  pthread_rwlock_init(&rwlock,NULL);
  pthread_t writer1, writer2, reader1, reader2, reader3, reader4, reader5, reader6;
  pthread_create(&writer1,NULL,lock_writer,"writer1");
  pthread_create(&writer2,NULL,lock_writer,"writer2");
  sleep(3);
  pthread_create(&reader1,NULL,lock_reader,"reader1");
  pthread_create(&reader2,NULL,lock_reader,"reader2");
  pthread_create(&reader3,NULL,lock_reader,"reader3");
  pthread_create(&reader4,NULL,lock_reader,"reader4");
  pthread_create(&reader5,NULL,lock_reader,"reader5");
  pthread_create(&reader6,NULL,lock_reader,"reader6");

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
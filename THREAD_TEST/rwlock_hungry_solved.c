#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
//写饥饿解决方案（对照 rwlock_write_hungry.c 读者优先版）
//根源：glibc 读写锁默认"读者优先"，读线程持锁或写者等待时，新到的读线程仍可插队获得读锁，
//      只要读流不断，写线程就永远排不上 → 写饥饿。
//方案：把锁属性改为"写者优先"，一旦有写者在等待写锁，之后到达的读线程会被阻塞排队在写者后面，
//      读线程无法再无限插队，写者的等待时间有界。

pthread_rwlock_t rwlock;
int shared_data=0;

void *lock_reader(void *argv)
{
  pthread_rwlock_rdlock(&rwlock);//读锁是共享锁：多个读线程可以同时持有
  printf("this is %s,value is %d \n",(char *)argv,shared_data);
  sleep(1);                      //持锁休眠1s，放大"读者持锁期间写者被阻塞"的效果

   pthread_rwlock_unlock(&rwlock);//释放读锁，唤醒排队中的写者/读者


}
void *lock_writer(void *argv)
{
  pthread_rwlock_wrlock(&rwlock);//写锁是排他锁：持锁期间其它读、写线程全部阻塞
  int tmp=shared_data+1;
  shared_data=tmp;
  printf("this is %s ,shared_data++\n",(char*)argv);

  pthread_rwlock_unlock(&rwlock);//写者优先：释放时若有写者在等待，新到达的读者不能再插到它前面
}
int main()
{
  pthread_rwlockattr_t attr; //读写锁属性对象，用来设置锁的"偏好"（kind）
  pthread_rwlockattr_init(&attr);

  //设置优先级 写者优先
  //① 默认属性是 PTHREAD_RWLOCK_PREFER_READER_NP（读者优先）：写者等待时新读者仍可插队 → 写饥饿；
  //② glibc 会忽略 PTHREAD_RWLOCK_PREFER_WRITER_NP（man 页说明：POSIX 允许写者递归获取读锁，
  //   严格写优先必然死锁），因此必须用 NONRECURSIVE 版：以"写者不再递归加锁"换取真正的写者优先；
  //③ _NP = Non-Portable，仅适用于 glibc/Linux。
  pthread_rwlockattr_setkind_np(&attr,PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);

  pthread_rwlock_init(&rwlock,&attr); //用带属性的方式初始化锁（属性在此时被复制进锁）
  pthread_rwlockattr_destroy(&attr);  //锁初始化完成后属性对象即可销毁

  pthread_t writer1,writer2,reader1,reader2,reader3,reader4,reader5,reader6;
  //pthread_create 创建线程后立即返回；8 个线程谁先执行、谁先拿到锁由调度器决定，
  //因此输出顺序每次运行可能不同（只有"写者开始等待后新读者不能插队"是确定的）。
  pthread_create(&writer1,NULL,lock_writer,"writer1");
  pthread_create(&reader1,NULL,lock_reader,"reader1");
  pthread_create(&reader2,NULL,lock_reader,"reader2");
  pthread_create(&reader3,NULL,lock_reader,"reader3");
  pthread_create(&writer2,NULL,lock_writer,"writer2");
  pthread_create(&reader4,NULL,lock_reader,"reader4");
  pthread_create(&reader5,NULL,lock_reader,"reader5");
  pthread_create(&reader6,NULL,lock_reader,"reader6");

  //join：主线程在此阻塞等待各线程结束，最终 shared_data 应为 2（两个写者各 +1）
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
//【实测观察与结论】
//1) 写者优先生效：writer2 不再像读者优先版(rwlock_write_hungry.c)那样总是最后一个执行，
//   而是插进队列中间 —— 在它之后才尝试加锁的读者被挡在它后面排队，写者等待时间有界。
//2) 不是"写者绝对第一"：在写者开始等待之前就已经持锁/先排队的读者仍然先执行；
//   偏好只作用于"写者开始等待之后到达的读者"，也不保证严格 FIFO。
//3) 代价：若写操作持续不断，读者反过来会被压后 —— 偏好只是把饥饿风险换到了另一侧。
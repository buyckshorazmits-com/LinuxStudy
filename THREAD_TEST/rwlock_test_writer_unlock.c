#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

pthread_rwlock_t rwlock;
int share_data=0;

void *lock_reader(void *argv)
{
  pthread_rwlock_rdlock(&rwlock);
  printf("this is%s ,value is %d \n",(char *)argv,share_data);

  pthread_rwlock_unlock(&rwlock);
}
void *lock_writer(void *argv)
{
  int tmp= share_data+1;

  sleep(1);
  share_data=tmp;
  printf("this is %s,share_data++\n",(char *)argv);

}
int main()
{
  pthread_rwlock_init(&rwlock,NULL);
  pthread_t writer1, writer2, reader1, reader2, reader3, reader4, reader5, reader6;
  
}
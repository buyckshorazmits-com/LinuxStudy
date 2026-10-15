#include <stdio.h>
#include <glib.h>      // GLib 线程池 API (GThreadPool)
#include <unistd.h>    // sleep()
#include <stdlib.h>    // malloc() / free()

//任务函数: worker 线程为每个任务调用一次
// data     = 本次任务的私有数据(push 时传入的 tmp)
// usr_data = 创建池时传入的共享数据(这里是 NULL, 没用上)
void task_func(gpointer data,gpointer usr_data)
{
  int  task_num=*(int*) data; // 读出任务号(唯一一次解引用, 之后不再碰 data)
  free(data);// free(data) 释放的正是主线程 malloc 出来的那块
  printf("Executing task is %d......\n",task_num);

  sleep(1); // 模拟 1 秒工作量
  printf("Task %d complete\n",task_num);
}
int main()
{
 //创建线程池: 参数=任务函数, 共享数据(NULL), 最多 5 线程, 独占模式(TRUE), 错误(NULL)
 GThreadPool *thread_pool=g_thread_pool_new(task_func,NULL,5,TRUE,NULL);
 
 //向线程池里添加任务
 for(int i=0;i<10;i++)
 {
  int *tmp=malloc(sizeof(int));  // 每个任务单独堆分配, 不能传 &i
  *tmp=i+1;
  g_thread_pool_push(thread_pool,tmp,NULL);//向线程池中加入数据  添加之后任务随时开跑, 池满则排队

 }
  //等待所有任务完成
  g_thread_pool_free(thread_pool,FALSE,TRUE); // FALSE=不丢弃排队任务; TRUE=等全部跑完才返回

  printf("All task complete!\n");
}
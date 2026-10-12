#include<stdio.h>      
#include<pthread.h>    
#include<stdlib.h>     
#include<unistd.h>     
#include<sys/mman.h>   //shm_open, mmap, munmap, shm_unlink(共享内存全家桶)
#include<sys/stat.h>   //文件状态/权限相关
#include<sys/types.h>  //pid_t 等基础类型
#include<sys/wait.h>   //waitpid
#include<fcntl.h>      //O_CREAT, O_RDWR 标志
/*============================================================================
  没有加入互斥的条件, 存在竟态条件的例程(进程间共享内存版)
  场景: 父子进程通过共享内存操作同一个 int 变量, 各自做一次 +1, 期望结果 = 2

  为什么最终是 1 而不是 2?
    "读值 → 加1 → 写回"是三步, 不是原子操作:
      父: 读 0 →(sleep 放大窗口)→ 写回 1
      子: 读 0 →(sleep 放大窗口)→ 写回 1
    双方都先读到 0, 又各写回 1, 互相覆盖 → 丢掉一次 +1(丢更新)
    "竞态条件"就是这么来的: 多个执行流不保护地操作同一数据, 结果取决于谁先谁后

  代码路线:
    ① 共享内存三件套: shm_open → ftruncate → mmap(让父子访问同一块内存)
    ② fork 后父子各做一次无保护的"读-加1-写回" → 竞态/丢更新
    ③ 收尾: munmap + close + shm_unlink

  下一版: 把初始值为 1 的无名信号量放进共享内存(sem_init 第二参=1 表示进程间共享),
          用 sem_wait/sem_post 把"读-加1-写回"保护起来, 结果才会稳定为 2
  ==========================================================================*/
int main()
{
  //共享内存对象的名字: 相当于 /dev/shm/ 下的一个"文件名"(ls /dev/shm 能看到)
  
  char *shm_value_name="unnamed_sem_shm_value";
   //创建一个共享内存对象
   //共享内存 = 独立于进程的一块公共内存，各进程把它映射进自己的地址空间后，就能一起读写同一份数据。
   //shm_open 用法类比 open():
   //  O_CREAT: 不存在就创建;  O_RDWR: 可读可写;  0666: 权限
   //  返回 value_fd 描述符(给后面的 ftruncate/mmap 用), 失败返回 -1
  int value_fd=shm_open(shm_value_name,O_CREAT | O_RDWR,0666);
  //调整内存对象的大小
  //注意: 新建的共享内存对象大小是 0, 必须显式设定!
  //      不设定就直接映射, 一访问会收到 SIGBUS 信号, 进程当场崩溃
  //这里设为 sizeof(int)=4 字节, 正好装一个 int
  ftruncate(value_fd,sizeof(int));
  //将内存对象的大小映射到共享内存区域
  //六个参数: ①起址( NULL=由内核挑空闲地址 )  ②长度( 4 字节 )
  //         ③保护权限( PROT_READ|PROT_WRITE = 可读可写, 与 open 的 O_RDWR 对应 )
  //         ④类型( MAP_SHARED = 关键! 写操作真正落到共享对象上, 别的进程看得见;
  //                 若换成 MAP_PRIVATE 就变成各写各的, 演示不出共享 )
  //         ⑤映射哪个对象( value_fd )  ⑥从偏移 0 开始
  //返回的 value 是 int*: 从此 *value 读写的就是共享内存(失败返回 MAP_FAILED, 不是 NULL)
  int *value=mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED,value_fd,0);

  //初始化共享变量的值
  //注意时机: 必须在 fork 之前初始化 —— 保证父子两个进程看到的都是 0
   *value=0;

   //fork 创建子进程: 从这行起代码"一分为二", 父子两个进程都从同一位置继续往下跑
   //  父进程: 返回子进程的 pid(>0) → 走下面 if(pid>0)
   //  子进程: 返回 0              → 走 else if(pid==0)
   //  失败:   返回 -1             → 走最后的 else
   int pid=fork();

  if(pid>0)   //父进程分支: fork 在父进程里返回子进程的 pid(>0)
  {
    //【1读-加1】从共享内存读出值(此刻是 0), +1 后暂存在本地变量 tmp
    //"读-加1-写回"整体不是原子操作, 这就是竞态的根源
    int tmp=*value+1;
    sleep(1);   //故意睡 1 秒"放大窗口": 让父子都完成"读"之后, 才开始"写回"

    //【2写回】把 tmp 写回共享内存
    //父子各写回一个 1, 谁后写谁覆盖对方 → 少加了一次(丢更新)
    *value=tmp;
    //父进程等待子进程运行完毕(阻塞 + 回收, 避免子进程变僵尸)
    //保证下一句 printf 时, 子进程一定已经完全结束
    waitpid(pid,NULL,0);

    //打印"最终"值: 正确应为 2, 实际输出 1 —— 丢掉了一次 +1
    printf("this is father,children was finished\n");
    printf("the final value is %d\n",*value);

  }
  else if(pid==0)
  {
    //和父进程完全相同的"读-加1-写回", 只是不打印
    //双方都先读到 0, 又都写回 1, 互相覆盖 → 丢更新
    int tmp = *value + 1;
        sleep(1);   //同样睡 1 秒放大竞态窗口
        *value = tmp;
        //子进程到此结束(不打印), 接着走下面公共的清理代码, 再退出

  }
  else
  {
    //fork 失败(返回 -1)才会走到这里: 打印失败的具体原因(如进程数达上限)
    perror("fork");
  }


// 无论父子进程都应该解除共享内存的映射，并关闭共享对象的文件描述符
// 为什么父子都要做: 映射、fd 都是 fork 时复制出来的"副本", 各进程各一份, 各清各的
// (进程退出时内核也会自动兜底释放, 但显式清理是好习惯; 长驻程序必须显式清)
// munmap: 把这段内存从本进程的地址空间拆掉; 之后 value 是野指针, 不能再解引用
if(munmap(value,sizeof(int))==-1)
{
  perror("munmap value");   //失败返回 -1, perror 会把具体原因打到 stderr
}
// close: 关闭文件描述符
// 冷知识: mmap 成功后映射与 fd 已"解绑", close 不影响 value 继续读写
if(close(value_fd)==-1)
{
  perror("close value");
}

 // 如果调用时别的进程仍在使用共享对象，则等待所有进程释放资源后，才会销毁相关资源。
    // shm_unlink只能调用一次,这里在父进程中调用shm_unlink
    // shm_unlink 删的是"名字"(类比 rm), 不是立刻销毁内存:
    //   已打开/已映射它的进程照常使用, 等最后一个使用者 munmap/close 后才真正释放
    // 名字全局唯一, 删第二次会报 No such file or directory, 所以只删一次;
    // 这里父进程已 waitpid 过, 子进程早已退出 → 名字删掉后对象立即彻底销毁
    // (若程序被 Ctrl+C/强杀, 这里来不及执行, 名字会残留在 /dev/shm, 可 rm 手动清理)
if(pid>0)
{
  if(shm_unlink(shm_value_name)==-1)
  {
    //走到这行说明 unlink 失败了(比如名字已不存在), 打印具体原因
    perror("father shm_unlink shm_value_name");
  }

}
//父进程(打印、清理、unlink 完)和子进程(清理完)都执行到这里: main 返回, 各自进程结束
return 0;

}
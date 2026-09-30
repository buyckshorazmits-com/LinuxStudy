#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>

int main()
{
  char *share;
  pid_t   pid;
  char shmName[100]={0};
  sprintf(shmName,"/letter%d",getpid());
  int fd;
  // 共享内存对象的文件标识符
  fd = shm_open(shmName,O_CREAT | O_RDWR,0644);//开启一块内存共享对象
  /**/
  if(fd<0)
  {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  //将该区域扩充到100字节大小
  ftruncate(fd,100);
  share=mmap(NULL,100,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
  // 以读写方式映射该区域到内存，并开启父子共享标签 成功时,返回映射区域的起始地址,可以像操作普通内存那样使用这个地址进行读写
  //把 fd 所代表的那 100 字节共享内存对象，映射到当前进程的虚拟地址空间中。
  if(share==MAP_FAILED)//映射失败标志
  {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
    // 映射区建立完毕,关闭读取连接 注意不是删除
    close(fd);
    // 创建子进程
    pid =fork();
    if(pid == 0)
    {
      strcpy(share,"hello world\n");
      printf("新学员%d完成回信\n",getpid());
      exit(0); // 子进程写完回信后退出
    }
    else 
    {
      // 等待子进程写完回信并结束
      wait(NULL);
      printf("老学员%d看到新学员%d的回信内容：%s",getpid(),pid,share);

      int ret=munmap(share,100);
      if(ret==-1)
      {
         perror("munmap");
         exit(EXIT_FAILURE);
      }

    }
     shm_unlink(shmName);
     return 0;
}

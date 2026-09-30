#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<string.h>
#include<sys/wait.h>
 int main()
 {
  char *share;
  pid_t pid;
  char shmName[100]={0};
  sprintf(shmName,"/letter%d",getpid());
  printf("shmName=%s\n",shmName);
  int fd;
  fd=shm_open(shmName,O_CREAT | O_RDWR,0644);
  if(fd<0)
  {
    perror("共享内存对象开启失败\n");
    exit(EXIT_FAILURE);
  }
 while(1);

  return 0;
  

 }
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>
//清空testfile.txt。
int main()
{
   FILE *file=fopen("/home/linux/vscoding/Scoket/testfile.txt","w");
   //所以“清空”这个行为是 "w" / "w+" 这两个模式特有的，不是 fopen 函数本身的属性。
   //fopen()会清楚文本文件以前的内容
   if(file==NULL)
   {
    perror("Failed to open file");

    return 1;
   }

   //设置文件为行缓冲模式
   if(setvbuf(file,NULL,_IOLBF,0)!=0)
   {
    perror("Failed to set  buffer mode");
    return 1;
   }

   fprintf(file,"hello");

   char *argv[]={"/usr/bin/ping","-c","1","www.baidu.com",NULL};
   char *envp[]={NULL};

   execve("/usr/bin/ping",argv,envp);
   perror("execve");

   return 0;

}
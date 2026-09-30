#include<stdio.h>
int main()
{
  FILE *ioFile=fopen("io.text","r");

  if(ioFile==NULL)
  {
    printf("文件打开失败,不能打开不存在的文件\n");
    
  }
  else
    {
      printf("文件打开成功\n");

    }
    /*
    char *__restrict __s: 接收读取的数据字符串
    int __n: 能够接收数据的长度
    FILE *__restrict __stream: 需要读取的文件
    return: 成功返回字符串 失败返回NULL(可以直接用于while)
    fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
    */

    char buffer[100];
     while(fgets(buffer,sizeof(buffer),ioFile))
     {
      printf("%s",buffer);//每次读一行,而不是一次把整个文件读进来。
     }  
    int result=fclose(ioFile);
    if(result !=0)
    {
      printf("文件关闭失败\n");
      return 1;
    }
    return 0;
}
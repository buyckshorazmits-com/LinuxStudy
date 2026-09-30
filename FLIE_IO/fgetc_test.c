#include<stdio.h>
int main()
{
  //打开文件
  char *filename ="io.text";
  FILE *ioFile=fopen(filename,"r");

  if(ioFile==NULL)
  {
    printf("文件打开失败,不能读取不存在的文件\n");
    return 1;
  }
  else
  {
    printf("文件打开成功\n");
  }
  /*
    FILE *__stream: 需要读取的文件
    return： 读取的一个字节 到文件结尾或出错返回EOF
    int fgetc (FILE *__stream)
    */

    int c = fgetc(ioFile);
    while (c != EOF)
    {
      printf("%c", c);   // 或 putchar(c) 这里的%c c要小写
      c = fgetc(ioFile);
    }
    
    int result=fclose(ioFile);

    if(result!=0)
    {
      printf("文件关闭失败\n");
      return 1;
    }
    return 0;

}
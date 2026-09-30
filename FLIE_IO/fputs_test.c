#include<stdio.h>
int main()
{
  /* 打开文件
    char *__restrict __filename: 字符串表示要打开文件的路径和名称
    char *__restrict __modes: 字符串表示访问模式
        (1)"r": 只读模式 没有文件打开失败
        (2)"w": 只写模式 存在文件写入会清空文件,不存在文件则创建新文件
        (3)"a": 只追加写模式 不会覆盖原有内容 新内容写到末尾，如果文件不存在则创建
        (4)"r+": 读写模式 文件必须存在 写入是从头一个一个覆盖
        (5)"w+": 读写模式 可读取,写入同样会清空文件内容，不存在则创建新文件
        (6)"a+": 读写追加模式 可读取,写入从文件末尾开始，如果文件不存在则创建
    return: FILE * 结构体指针 表示一个文件
    FILE *fopen (const char *__restrict __filename,
            const char *__restrict __modes)
    */
  char *filename="io.text";
  FILE *ioFile =fopen(filename,"a+");
/*
const char *__restrict __s:需要写入的字符串
FILE *__restrict __stream：需要写入的文件位置
return :成功返回非负整数(0,1) 失败返回EOF
int fputs (const char *__restrict __s, FILE *__restrict __stream);
*/
  int putCS=fputs("love lettet\n",ioFile);
  if(putCS==EOF)
  {
    printf("字符串写入失败\n");
  }
  else
  {
    printf("字符串写入%d成功\n",putCS);
  }

  int result=fclose(ioFile);
  if(result!=0)
  {
    printf("文件关闭失败\n");
  }

  return 0;

}